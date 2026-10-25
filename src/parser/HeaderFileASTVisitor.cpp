#include "HeaderFileASTVisitor.h"
#include "HeaderCollectPPCallback.h"
#include "functionLibrary/PropertyFilterLibrary.h"
#include "functionLibrary/MethodFilterLibrary.h"

#include <clang/Basic/SourceManager.h>
#include <filesystem>

#include "debug/SuzzaLog.h"

namespace suz
{
	HeaderFileASTVisitor::HeaderFileASTVisitor(
		std::unordered_map<std::string, std::vector<clang::SourceLocation>>& MacroExpandPath,
		std::unordered_map<std::string, unsigned int>& MacroExpandLine,
		std::unordered_map<std::string, ClassInfo>& ClassInfoList
	)
		: m_macroExpandPath(MacroExpandPath), m_macroExpandLine(MacroExpandLine), m_classInfo(ClassInfoList)
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

		PropertyFilterLibrary::ConstructClassInfo(RecordDecl, sourceManager, m_macroExpandLine, m_classInfo);

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
}