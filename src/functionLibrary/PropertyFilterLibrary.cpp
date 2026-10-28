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
	EPrimitiveType MakePrimitiveTypeEnum(const clang::ASTContext& Context, const clang::QualType& CanonicalType, const clang::BuiltinType& Builtin);

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
		MethodFilterLibrary::CollectGetterSetterMethod(RecordDecl, OutClassInfo);

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
			return;
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
					if (annotateAttr->getAnnotation() != "property:") continue;

					FieldInfo fieldInfo{};
					/* 读取注解内的参数 */
					ReadAnnotationArgs(annotateAttr, fieldInfo);

					clang::QualType fieldType = field->getType();
					/* 构建字段的枚举类型 */
					GainPropertyTypeEnum(fieldType, RecordDecl->getASTContext(), fieldInfo);

					std::string fieldName = field->getNameAsString();			// 获取字段名称
					std::string fieldTypeStr = fieldType.getAsString();			// 获取字段类型名称

					fieldInfo.name = fieldName;
					fieldInfo.typeName = fieldTypeStr;
					fieldInfo.ownerClassName = RecordDecl->getNameAsString();	// 设置所属类名
					
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
				if (enumSeparate != std::string::npos)
				{
					llvm::StringRef enumName = argName.substr(0, enumSeparate);
					llvm::StringRef enumValue = argName.substr(enumSeparate + 2);

					EnumParseLibrary::ReadEnumStructInfo(enumName, enumValue, OutFieldInfo);
				}
			}
		}
	}

	void PropertyFilterLibrary::GainPropertyTypeEnum(const clang::QualType& QType, const clang::ASTContext& Context, FieldInfo& OutFieldInfo)
	{
		const clang::QualType canonicalType = QType.getCanonicalType();
		const clang::Type* type = canonicalType.getTypePtrOrNull();
		if (!type) return;

		/* 优先判断容器类型 */
		if (const clang::CXXRecordDecl* recordDecl = type->getAsCXXRecordDecl())
		{
			const auto* specDecl = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(recordDecl);
			if (!specDecl) return;

			ParseContainerType(specDecl, OutFieldInfo);
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

			case clang::BuiltinType::Float:
				OutFieldInfo.primitiveType = EPrimitiveType::Float; break;

			case clang::BuiltinType::Double:
				OutFieldInfo.primitiveType = EPrimitiveType::Double; break;

			default:
				if (builtinType->isInteger())
				{
					OutFieldInfo.primitiveType = MakePrimitiveTypeEnum(Context, canonicalType, *builtinType);
					if (OutFieldInfo.primitiveType == EPrimitiveType::Unknown)
					{
						SUZZA_LOG_INFO("未识别的整数类型!: ", canonicalType.getAsString());
					}
				}
				else
				{
					OutFieldInfo.primitiveType = EPrimitiveType::Unknown;
					SUZZA_LOG_INFO("未识别的内建类型!: ", canonicalType.getAsString());
				}
			}
		}
		else
		{
			OutFieldInfo.category = ETypeCategory::Unknown;
		}
	}

	void PropertyFilterLibrary::ParseContainerType(const clang::ClassTemplateSpecializationDecl* SpecialDecl, FieldInfo& OutFieldInfo)
	{
		const clang::ClassTemplateDecl* templateDecl = SpecialDecl->getSpecializedTemplate();
		if (!templateDecl) return;

		std::string containerName = templateDecl->getQualifiedNameAsString();
		if (containerName.starts_with("glm::"))
		{
			HandleInGlmContainer(containerName, SpecialDecl->getTemplateArgs(), OutFieldInfo);
		}
		else if (containerName.starts_with("std::"))
		{
			HandleInStdContainer(containerName, OutFieldInfo);
		}
	}

	void PropertyFilterLibrary::HandleInGlmContainer(const std::string& ContainerName, const clang::TemplateArgumentList& ArgList, FieldInfo& OutFieldInfo)
	{
		/* 设置lambda获取模板变量的辅助函数 */
		auto tryGetIntegralArg = [&ArgList](uint8 index, uint8& outValue) ->bool {
			if (index >= ArgList.size()) return false;

			const clang::TemplateArgument& value = ArgList.get(index);
			if (value.getKind() != clang::TemplateArgument::Integral) return false;

			outValue = value.getAsIntegral().getExtValue();
			return true;
			};

		if (ContainerName == "glm::vec")
		{
			uint8 value;
			if (!tryGetIntegralArg(0, value))
			{
				return;
			}

			if (value == 3)
			{
				OutFieldInfo.category = ETypeCategory::Container;
				OutFieldInfo.containerType = EContainerType::Vector3;
			}
			else if (value == 4)
			{
				OutFieldInfo.category = ETypeCategory::Container;
				OutFieldInfo.containerType = EContainerType::Vector4;
			}
			else
			{
				OutFieldInfo.category = ETypeCategory::Unknown;
				OutFieldInfo.containerType = EContainerType::Unknown;
			}
		}
		else if (ContainerName == "glm::qua")
		{
			OutFieldInfo.category = ETypeCategory::Container;
			OutFieldInfo.containerType = EContainerType::Quat;
		}
		else if (ContainerName == "glm::mat")
		{
			uint8 column, raw;
			if (!tryGetIntegralArg(0, column) || !tryGetIntegralArg(1, raw)) return;

			if (column == raw == 3)
			{
				OutFieldInfo.category = ETypeCategory::Container;
				OutFieldInfo.containerType = EContainerType::Matrix3;
			}
			else if (column == raw == 4)
			{
				OutFieldInfo.category = ETypeCategory::Container;
				OutFieldInfo.containerType = EContainerType::Matrix4;
			}
			else
			{
				OutFieldInfo.category = ETypeCategory::Unknown;
				OutFieldInfo.containerType = EContainerType::Unknown;
			}
		}
	}

	void PropertyFilterLibrary::HandleInStdContainer(const std::string& ContainerName, FieldInfo& OutFieldInfo)
	{
		if (ContainerName == "std::basic_string")
		{
			OutFieldInfo.category = ETypeCategory::Container;
			OutFieldInfo.containerType = EContainerType::String;
		}
		else if (ContainerName == "std::vector")
		{
			OutFieldInfo.category = ETypeCategory::Container;
			OutFieldInfo.containerType = EContainerType::Vector;
		}
		else if (ContainerName == "std::map")
		{
			OutFieldInfo.category = ETypeCategory::Container;
			OutFieldInfo.containerType = EContainerType::Map;
		}
		else if (ContainerName == "std::set")
		{
			OutFieldInfo.category = ETypeCategory::Container;
			OutFieldInfo.containerType = EContainerType::Set;
		}
	}

	EPrimitiveType MakePrimitiveTypeEnum(const clang::ASTContext& Context, const clang::QualType& CanonicalType, const clang::BuiltinType& Builtin)
	{
		/* 判断有无符号的整数类型(包括char类型) */
		const bool bIsSignedInteger = Builtin.isSignedInteger();

		switch (Context.getIntWidth(CanonicalType))
		{
		case 8: return bIsSignedInteger ? EPrimitiveType::Int8 : EPrimitiveType::UInt8;
		case 16: return bIsSignedInteger ? EPrimitiveType::Int16 : EPrimitiveType::UInt16;
		case 32: return bIsSignedInteger ? EPrimitiveType::Int32 : EPrimitiveType::UInt32;
		case 64: return bIsSignedInteger ? EPrimitiveType::Int64 : EPrimitiveType::UInt64;
		default:
			return EPrimitiveType::Unknown;
		}
	}
}