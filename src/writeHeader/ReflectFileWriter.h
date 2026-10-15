#pragma once

#include "struct/ReflectStructInfo.h"

#include <string>
#include <unordered_map>

namespace suz
{
	class ReflectFileWriter
	{
	public:
		void WriteReflectSourceFile();

		void SetOutFilePath(const std::string& OutFilePath);
		void SetProjectPath(const std::string& ProjectPath);

		void SetClassInfo(const std::unordered_map<std::string, ClassInfo>& ClassInfoData);

	private:
		void WriteHeaderFile(const std::pair<std::string, ClassInfo>& ClassData);
		void WriteSourceFile(const std::pair<std::string, ClassInfo>& ClassData);

		std::string GenerateRegisterHeaderFileStr();

#pragma region GenericSource
		std::string GenerateRegisterSourceFileHeaderStr();

		std::string GenerateRegisterSourceFileFunctionBeginStr();
		std::string GenerateRegisterSourceFileFunctionEndStr();

		std::string GenerateRegisterSourceFileIncludeStr(const std::pair<std::string, ClassInfo>& ClassData);
		std::string GenerateRegisterSourceFileFunctionBodyStr(const std::pair<std::string, ClassInfo>& ClassData);

#pragma endregion

		std::string m_cacheOutFilePath = {};
		std::string m_cacheProjectPath = {};

		std::unordered_map<std::string, ClassInfo> m_classInfo = {};
	};
}