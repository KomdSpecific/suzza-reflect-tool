#pragma once

#include "struct/ReflectStructInfo.h"

#include <clang/Lex/PPCallbacks.h>
#include <unordered_set>

namespace suz
{
	class HeaderCollectPPCallback : public clang::PPCallbacks
	{
	public:
		HeaderCollectPPCallback(
			std::unordered_map<std::string, std::vector<clang::SourceLocation>>& OutMacroMap,
			std::unordered_map<std::string, unsigned int>& OutMacroExpandLine,
			clang::SourceManager& SourceManager)
			: m_macroExpandList(OutMacroMap), m_macroExpandLine(OutMacroExpandLine), m_sourceManager(SourceManager)
		{
		}

		void MacroExpands(
			const clang::Token& MacroNameTok,
			const clang::MacroDefinition& MD,
			clang::SourceRange Range,
			const clang::MacroArgs* Args
		) override;

		std::unordered_map<std::string, std::vector<clang::SourceLocation>>& m_macroExpandList;
		std::unordered_map<std::string, unsigned int>& m_macroExpandLine;
		clang::SourceManager& m_sourceManager;
	};
}