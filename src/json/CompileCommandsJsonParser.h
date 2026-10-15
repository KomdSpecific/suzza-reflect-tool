#pragma once

#include <string>

#include <llvm/Support/JSON.h>
#include <llvm/Option/ArgList.h>

namespace suz
{
	class CompileCommandsJsonParser
	{
	public:
		static llvm::json::Array ParseCompileCommandsJson(const std::string& FilePath);

		static std::vector<std::string> SplitCommandLine(const llvm::StringRef& CommandStr);
	};
}