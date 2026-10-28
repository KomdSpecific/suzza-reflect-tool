#include "HeaderCollectPPCallback.h"

#include <clang/Basic/SourceManager.h>
#include <clang/Basic/IdentifierTable.h>
#include <clang/Lex/Token.h>
#include <clang/Lex/MacroInfo.h>
#include <llvm/Support/Path.h>

#include <filesystem>

#include "debug/SuzzaLog.h"

namespace suz
{
	void HeaderCollectPPCallback::MacroExpands(const clang::Token& MacroNameTok, const clang::MacroDefinition& MD, clang::SourceRange Range, const clang::MacroArgs* Args)
	{
		if (MacroNameTok.getIdentifierInfo()->getName() == "SUZZA_REFLECT_CLASS")
		{
			clang::SourceLocation macroLocation = MacroNameTok.getLocation();
			/* 缓存宏被调用的文件位置 */
			std::string macroFilePath = m_sourceManager.getFilename(macroLocation).str();
			std::string genericPath = std::filesystem::path(macroFilePath).generic_string();

			unsigned int macroLine = m_sourceManager.getPresumedLineNumber(macroLocation);
			/*SUZZA_LOG_INFO("file: ", genericPath, " line: ", macroLine);*/
			m_macroExpandLine[genericPath] = macroLine;

			auto it = m_macroExpandList.find(genericPath);
			/* 当前类与其他类存储在同一源文件中 */
			if (it != m_macroExpandList.end())
			{
				it->second.push_back(macroLocation);
			}
			else
			{
				m_macroExpandList[genericPath] = { macroLocation };
			}
		}
	}
}