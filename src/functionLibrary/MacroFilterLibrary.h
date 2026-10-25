#pragma once

#include "SuzzaTypes.h"

#include <string>
#include <unordered_map>

namespace clang
{
	class Token;
	class IdentifierInfo;
	class SourceManager;
	class MacroArgs;
}

namespace suz
{
	class MacroFilterLibrary
	{
	public:
		static void CollectMacroExpandLineNumber(
			const clang::Token& MacroNameTok,
			const std::string& MacroName,
			const clang::MacroArgs* Args,
			clang::SourceManager& SourceManager,
			std::unordered_map<unsigned int, std::string>& OutExpandLineMap
		);
	};
}