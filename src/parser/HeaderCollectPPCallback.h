#pragma once

#include "struct/ReflectStructInfo.h"

#include <clang/Lex/PPCallbacks.h>
#include <unordered_set>

namespace suz
{
	class HeaderCollectPPCallback : public clang::PPCallbacks
	{
	public:
		HeaderCollectPPCallback(std::unordered_map<std::string, std::vector<clang::SourceLocation>>& OutMacroMap, clang::SourceManager& SourceManager)
			: m_macroExpandList(OutMacroMap), m_sourceManager(SourceManager)
		{
		}

		void MacroExpands(
			const clang::Token& MacroNameTok,
			const clang::MacroDefinition& MD,
			clang::SourceRange Range,
			const clang::MacroArgs* Args
		) override;

		std::unordered_map<std::string, std::vector<clang::SourceLocation>>& m_macroExpandList;
		clang::SourceManager& m_sourceManager;
	};
}