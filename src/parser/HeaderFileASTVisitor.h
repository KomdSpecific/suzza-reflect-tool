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
			std::unordered_map<std::string, unsigned int>& MacroExpandLine,
			std::unordered_map<std::string, ClassInfo>& ClassInfoList);

		bool VisitCXXRecordDecl(clang::CXXRecordDecl* RecordDecl);

		std::unordered_map<std::string, std::vector<clang::SourceLocation>>& m_macroExpandPath;
		std::unordered_map<std::string, unsigned int>& m_macroExpandLine;
		std::unordered_map<std::string, ClassInfo>& m_classInfo;

	private:
		bool HasTargetMacro(clang::CXXRecordDecl* RecordDecl, const clang::SourceManager& SourceManager);
	};
}