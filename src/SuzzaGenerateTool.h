#pragma once

#include "writeHeader/ReflectFileWriter.h"

#include <llvm/Support/JSON.h>

namespace suz
{
	class SuzzaGenerateTool
	{
	public:
		SuzzaGenerateTool() : m_reflectFileWriter(std::make_unique<ReflectFileWriter>()) {}

		bool Init(const std::string& OutFilePath, const std::string& ProjectPath, const std::string& ParseJsonPath);

		void Run();

		void WriteReflectFile();

	private:
		SuzzaGenerateTool(SuzzaGenerateTool&) = delete;
		SuzzaGenerateTool(const SuzzaGenerateTool&) = delete;
		SuzzaGenerateTool& operator=(SuzzaGenerateTool&) = delete;
		SuzzaGenerateTool& operator=(const SuzzaGenerateTool&) = delete;

		bool CollectHeaderFilePath(const llvm::json::Array& ParseArray);

		llvm::json::Array m_parseFileArr = {};

		std::unique_ptr<ReflectFileWriter> m_reflectFileWriter = {};
	};
}