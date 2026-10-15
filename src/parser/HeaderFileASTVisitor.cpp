#include "HeaderFileASTVisitor.h"
#include "HeaderCollectPPCallback.h"

#include <clang/Basic/SourceManager.h>
#include <filesystem>

#include "debug/SuzzaLog.h"

namespace suz
{
	HeaderFileASTVisitor::HeaderFileASTVisitor(
		std::unordered_map<std::string, std::vector<clang::SourceLocation>>& MacroExpandPath,
		std::unordered_map<std::string, ClassInfo>& ClassInfoList
	)
		: m_macroExpandPath(MacroExpandPath), m_classInfo(ClassInfoList)
	{
	}

	bool HeaderFileASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl* RecordDecl)
	{
		if (!RecordDecl->isThisDeclarationADefinition() || RecordDecl->isImplicit())
		{
			return true;
		}

		clang::SourceManager& sourceManager = RecordDecl->getASTContext().getSourceManager();
		clang::SourceLocation fileLocation = RecordDecl->getLocation();
		/* 第一层过滤: 系统头文件过滤 */
		if (sourceManager.isInSystemHeader(fileLocation)) return true;

		/* 第二层过滤: 检查类内部是否存在目标宏 */
		if (!HasTargetMacro(RecordDecl, sourceManager)) return true;

		ConstructClassInfo(RecordDecl, sourceManager);

		return true;
	}

	bool HeaderFileASTVisitor::HasTargetMacro(clang::CXXRecordDecl* RecordDecl, const clang::SourceManager& SourceManager)
	{
		if (m_macroExpandPath.empty()) return false;

		/* 获取类的完整源码范围 */
		clang::SourceRange sourceRange = RecordDecl->getSourceRange();
		
		std::string filePath = SourceManager.getFilename(RecordDecl->getLocation()).str();
		std::filesystem::path classPath(filePath);
		if (!std::filesystem::exists(classPath))
		{
			return false;
		}

		/* 遍历记录的所有宏展开位置 */
		for (const std::pair<std::string, std::vector<clang::SourceLocation>>& pair : m_macroExpandPath)
		{
			/* 确保宏和类在同一文件 */
			std::filesystem::path macroFilePath(pair.first);

			if (std::filesystem::exists(macroFilePath))
			{
				if (!std::filesystem::equivalent(macroFilePath, classPath))
				{
					continue;
				}
			}

			for (const clang::SourceLocation& macroLoc : pair.second)
			{
				if (SourceManager.isBeforeInTranslationUnit(sourceRange.getBegin(), macroLoc) &&
					SourceManager.isBeforeInTranslationUnit(macroLoc, sourceRange.getEnd()))
				{
					return true;
				}
			}
		}

		return false;
	}

	void HeaderFileASTVisitor::ConstructClassInfo(clang::CXXRecordDecl* RecordDecl, const clang::SourceManager& SourceManager)
	{
		std::string fullClassName = RecordDecl->getQualifiedNameAsString();
		
		ClassInfo classInfo{};
		classInfo.className = fullClassName;

		clang::SourceLocation fileLocation = RecordDecl->getLocation();
		if (fileLocation.isValid())
		{
			/* 获取类的文件路径 */
			std::string filePath = SourceManager.getFilename(fileLocation).str();
			classInfo.classPath = filePath;
		}

		/* 防重复集合 */
		std::unordered_set<const clang::CXXRecordDecl*> declSet{};

		ExtractFields(RecordDecl, SourceManager, classInfo, declSet);

		m_classInfo.insert({ fullClassName,classInfo });
	}

	void HeaderFileASTVisitor::ExtractFields(
		clang::CXXRecordDecl* RecordDecl,
		const clang::SourceManager& SourceManager,
		ClassInfo& OutClassInfo,
		std::unordered_set<const clang::CXXRecordDecl*>& VisitedClassSet,
		const std::string& ParentClassName)
	{
		/* 防止菱形继承导致死循环和重复提取 */
		if (!RecordDecl || VisitedClassSet.count(RecordDecl))
		{
			return;
		}

		VisitedClassSet.insert(RecordDecl);

		if (!ParentClassName.empty())
		{
			OutClassInfo.superClassName.push_back(ParentClassName);
		}

		/* 提取当前类自身的所有字段 */
		for (const clang::FieldDecl* field : RecordDecl->fields())
		{
			clang::QualType fieldType = field->getType();

			std::string fieldName = field->getNameAsString();			// 获取字段名称
			std::string fieldTypeStr = fieldType.getAsString();			// 获取字段类型名称

			FieldInfo fieldInfo{};
			fieldInfo.name = fieldName;
			fieldInfo.typeName = fieldTypeStr;
			OutClassInfo.fields.push_back(fieldInfo);
		}

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
}