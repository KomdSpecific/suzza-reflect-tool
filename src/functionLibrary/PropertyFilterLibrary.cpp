#include "PropertyFilterLibrary.h"
#include "MethodFilterLibrary.h"
#include "EnumParseLibrary.h"

#include <clang/AST/Attr.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Basic/SourceManager.h>
#include <filesystem>

#include "debug/SuzzaLog.h"

namespace suz
{
	void PropertyFilterLibrary::ConstructClassInfo(
		clang::CXXRecordDecl* RecordDecl,
		const clang::SourceManager& SourceManager,
		const std::unordered_map<std::string, unsigned int>& MacroExpandLine,
		std::unordered_map<std::string, ClassInfo>& OutClassInfoMap
	)
	{
		std::string fullClassName = RecordDecl->getQualifiedNameAsString();

		ClassInfo classInfo{};
		classInfo.className = fullClassName;

		clang::SourceLocation fileLocation = RecordDecl->getLocation();
		if (fileLocation.isValid())
		{
			/* 获取类的文件路径 */
			std::string filePath = std::filesystem::path(SourceManager.getFilename(fileLocation).str()).generic_string();
			classInfo.classPath = filePath;

			auto it = MacroExpandLine.find(filePath);
			if (it != MacroExpandLine.end())
			{
				classInfo.macroDefinedLine = it->second;
			}
		}

		/* 防重复集合 */
		std::unordered_set<const clang::CXXRecordDecl*> declSet{};

		ExtractFields(RecordDecl, SourceManager, classInfo, declSet);

		OutClassInfoMap.insert({ fullClassName,classInfo });
	}

	/* 递归提取字段 */
	void PropertyFilterLibrary::ExtractFields(
		clang::CXXRecordDecl* RecordDecl,
		const clang::SourceManager& SourceManager,
		ClassInfo& OutClassInfo,
		std::unordered_set<const clang::CXXRecordDecl*>& VisitedClassSet,
		const std::string& ParentClassName
	)
	{
		/* 防止菱形继承导致死循环和重复提取 */
		if (!RecordDecl || VisitedClassSet.count(RecordDecl))
		{
			return;
		}

		VisitedClassSet.insert(RecordDecl);

		/* 收集拥有注解的字段 */
		ReadHasAnnotationField(RecordDecl, OutClassInfo, ParentClassName);

		/* 收集指定变量的Getter和Setter函数 */
		MethodFilterLibrary::CollectGetterSetterMethod(RecordDecl, "Getter:", "Setter:", OutClassInfo);

		/* 递归遍历父类 */
		for (const clang::CXXBaseSpecifier& base : RecordDecl->bases())
		{
			if (base.getAccessSpecifier() != clang::AS_public)
			{
				continue;
			}

			/* 获取父类的Decl */
			clang::CXXRecordDecl* parentDecl = base.getType()->getAsCXXRecordDecl();
			if (!parentDecl || !parentDecl->isThisDeclarationADefinition())
			{
				continue;
			}

			/* 过滤系统头文件基类 */
			clang::SourceLocation parentLoc = parentDecl->getLocation();
			if (SourceManager.isInSystemHeader(parentLoc))
			{
				continue;
			}

			/* 递归遍历父类字段 */
			ExtractFields(parentDecl, SourceManager, OutClassInfo, VisitedClassSet, base.getType().getAsString());
		}
	}

	void PropertyFilterLibrary::ReadHasAnnotationField(clang::CXXRecordDecl* RecordDecl, ClassInfo& OutClassInfo, const std::string& ParentClassName)
	{
		bool isParentClass = !ParentClassName.empty();
		if (isParentClass)
		{
			OutClassInfo.superClassName.push_back(ParentClassName);
		}

		/* 提取当前类自身的所有字段 */
		for (clang::FieldDecl* field : RecordDecl->fields())
		{
			/* 遍历所有的字段注解 */
			for (clang::Attr* attr : field->attrs())
			{
				/* 如果注解合适则提取对应的字段 */
				if (const clang::AnnotateAttr* annotateAttr = clang::dyn_cast<clang::AnnotateAttr>(attr))
				{
					/* 获取注解内的固定字面量 */
					if (annotateAttr->getAnnotation() != "property:") return;

					FieldInfo fieldInfo{};
					/* 读取注解内的参数 */
					ReadAnnotationArgs(annotateAttr, fieldInfo);

					clang::QualType fieldType = field->getType();
					/* 构建字段的枚举类型 */
					GainPropertyTypeEnum(fieldType, fieldInfo);

					std::string fieldName = field->getNameAsString();			// 获取字段名称
					std::string fieldTypeStr = fieldType.getAsString();			// 获取字段类型名称

					fieldInfo.name = fieldName;
					fieldInfo.typeName = fieldTypeStr;
					if (isParentClass)
					{
						fieldInfo.parentClassName = ParentClassName;
					}
					//SUZZA_LOG_INFO("FieldName: ", fieldInfo.name, " AnnoString: ", annoStringRef.str());
					OutClassInfo.fields.push_back(fieldInfo);
				}
			}
		}
	}

	void PropertyFilterLibrary::ReadAnnotationArgs(const clang::AnnotateAttr* AnnotateAttr, FieldInfo& OutFieldInfo)
	{
		for (const clang::Expr* arg : AnnotateAttr->args())
		{
			if (const clang::StringLiteral* strExpr = clang::dyn_cast<clang::StringLiteral>(arg->IgnoreParenImpCasts()))
			{
				llvm::StringRef argName = strExpr->getString();

				usize enumSeparate = argName.find("::");
				llvm::StringRef enumName = argName.substr(0, enumSeparate);
				llvm::StringRef enumValue = argName.substr(enumSeparate + 2);

				EnumParseLibrary::ReadEnumStructInfo(enumName, enumValue, OutFieldInfo);
			}
		}
	}

	void PropertyFilterLibrary::GainPropertyTypeEnum(const clang::QualType& QType, FieldInfo& OutFieldInfo)
	{
		const clang::Type* type = QType.getTypePtrOrNull();
		if (!type) return;

		/* 优先判断STL容器 */
		if (const clang::TemplateSpecializationType* stlType = type->getAs<clang::TemplateSpecializationType>())
		{
			clang::TemplateDecl* tempDecl = stlType->getTemplateName().getAsTemplateDecl();
			if (!tempDecl) return;

			std::string containerName = tempDecl->getQualifiedNameAsString();
			if (containerName.empty()) return;

			llvm::ArrayRef<clang::TemplateArgument> argsArray = stlType->template_arguments();
			for (const clang::TemplateArgument& arg : argsArray)
			{
				std::string argName = arg.getAsType().getAsString();
			}
		}
		else if (type->isPointerType())
		{
			OutFieldInfo.category = ETypeCategory::Pointer;
		}
		else if (type->isReferenceType())
		{
			OutFieldInfo.category = ETypeCategory::Reference;
		}
		else if (type->isArrayType())
		{
			OutFieldInfo.category = ETypeCategory::Array;
		}
		else if (type->isRecordType())
		{
			OutFieldInfo.category = ETypeCategory::Struct;
		}
		else if (type->isEnumeralType())
		{
			OutFieldInfo.category = ETypeCategory::Enum;
		}
		else if (type->isBuiltinType())
		{
			OutFieldInfo.category = ETypeCategory::Primitive;
			const clang::BuiltinType* builtinType = clang::cast<clang::BuiltinType>(type);
			
			switch (builtinType->getKind())
			{
			case clang::BuiltinType::Bool:
				OutFieldInfo.primitiveType = EPrimitiveType::Bool; break;
			case clang::BuiltinType::SChar:
				OutFieldInfo.primitiveType = EPrimitiveType::Int8; break;
			case clang::BuiltinType::UChar:
				OutFieldInfo.primitiveType = EPrimitiveType::UInt8; break;
			case clang::BuiltinType::Short:
				OutFieldInfo.primitiveType = EPrimitiveType::Int16; break;
			case clang::BuiltinType::UShort:
				OutFieldInfo.primitiveType = EPrimitiveType::UInt16; break;
			case clang::BuiltinType::Int:
				OutFieldInfo.primitiveType = EPrimitiveType::Int32; break;
			case clang::BuiltinType::UInt:
				OutFieldInfo.primitiveType = EPrimitiveType::UInt32; break;
			case clang::BuiltinType::LongLong:
				OutFieldInfo.primitiveType = EPrimitiveType::Int64; break;
			case clang::BuiltinType::ULongLong:
				OutFieldInfo.primitiveType = EPrimitiveType::UInt64; break;
			case clang::BuiltinType::Float:
				OutFieldInfo.primitiveType = EPrimitiveType::Float; break;
			default:
				OutFieldInfo.primitiveType = EPrimitiveType::Unknown; break;
			}
		}
		else
		{
			OutFieldInfo.category = ETypeCategory::Unknown;
		}
	}
}