#pragma once

#include "struct/ReflectStructInfo.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <unordered_map>
#include <unordered_set>

namespace suz
{
	class HeaderFileASTVisitor : public clang::RecursiveASTVisitor<HeaderFileASTVisitor>
	{
	public:
		HeaderFileASTVisitor(
			std::unordered_map<std::string, std::vector<clang::SourceLocation>>& MacroExpandPath,
			std::unordered_map<std::string, ClassInfo>& ClassInfoList);

		bool VisitCXXRecordDecl(clang::CXXRecordDecl* RecordDecl);

		std::unordered_map<std::string, std::vector<clang::SourceLocation>>& m_macroExpandPath;
		std::unordered_map<std::string, ClassInfo>& m_classInfo;

	private:
		bool HasTargetMacro(clang::CXXRecordDecl* RecordDecl, const clang::SourceManager& SourceManager);

		void ConstructClassInfo(clang::CXXRecordDecl* RecordDecl, const clang::SourceManager& SourceManager);

		/* µÝ¹éÌáÈ¡×Ö¶Î */
		void ExtractFields(
			clang::CXXRecordDecl* RecordDecl,
			const clang::SourceManager& SourceManager,
			ClassInfo& OutClassInfo,
			std::unordered_set<const clang::CXXRecordDecl*>& VisitedClassSet,
			const std::string& ParentClassName = {}
		);
	};
}