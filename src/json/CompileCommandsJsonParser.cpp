#include "CompileCommandsJsonParser.h"

#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/CommandLine.h>

#include "debug/SuzzaLog.h"

namespace suz
{
	llvm::json::Array CompileCommandsJsonParser::ParseCompileCommandsJson(const std::string& FilePath)
	{
		auto bufferResult = llvm::MemoryBuffer::getFile(FilePath);
		if (!bufferResult)
		{
			SUZZA_LOG_ERROR("Cannot open target json: %s", FilePath);
			return {};
		}

		llvm::StringRef jsonText = bufferResult->get()->getBuffer();

		auto parsed = llvm::json::parse(jsonText);
		if (!parsed)
		{
			SUZZA_LOG_ERROR("Json parse error!");
			return {};
		}

		llvm::json::Array arr = *parsed->getAsArray();
		if (arr.empty())
		{
			SUZZA_LOG_ERROR("JSON解析文件结果为空!");
			return {};
		}

		return arr;
	}

	std::vector<std::string> CompileCommandsJsonParser::SplitCommandLine(const llvm::StringRef& CommandStr)
	{
		llvm::BumpPtrAllocator alloc{};
		llvm::StringSaver saver(alloc);
		llvm::SmallVector<const char*, 64> argvRaw;
		llvm::cl::TokenizeWindowsCommandLine(CommandStr, saver, argvRaw);

		std::vector<std::string> args{};
		for (auto raw : argvRaw)
		{
			args.emplace_back(raw);
		}
		
		return args;
	}
}