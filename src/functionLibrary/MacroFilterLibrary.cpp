#include "MacroFilterLibrary.h"

#include <clang/Basic/SourceManager.h>
#include <clang/Basic/IdentifierTable.h>
#include <clang/Lex/MacroArgs.h>
#include <clang/Lex/Token.h>

namespace suz
{
	void MacroFilterLibrary::CollectMacroExpandLineNumber(
		const clang::Token& MacroNameTok,
		const std::string& MacroName,
		const clang::MacroArgs* Args,
		clang::SourceManager& SourceManager,
		std::unordered_map<unsigned int, std::string>& OutExpandLineMap
	)
	{
		if (MacroNameTok.getIdentifierInfo()->getName() == MacroName)
		{
			clang::SourceLocation macroLocation = MacroNameTok.getLocation();
			unsigned int macroLine = SourceManager.getPresumedLineNumber(macroLocation);

			if (Args)
			{
				
			}
		}
	}
}