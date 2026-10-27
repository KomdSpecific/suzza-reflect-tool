#include "ReflectFileWriter.h"
#include "GenerateReflectString.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "debug/SuzzaLog.h"

namespace suz
{
	void ReflectFileWriter::WriteReflectSourceFile()
	{
		/* 项目目录不存在, 直接返回 */
		if (!std::filesystem::exists(m_cacheProjectPath)) return;

		if (!std::filesystem::exists(m_cacheOutFilePath))
		{
			std::error_code err;
			std::filesystem::create_directories(m_cacheOutFilePath, err);

			if (err)
			{
				SUZZA_LOG_ERROR("创建目录失败! 错误信息: ", err);
				return;
			}
		}

		std::string registerHeaderPath = m_cacheOutFilePath + "/RegisterAllClass.h";
		std::ofstream registerHeaderOss(registerHeaderPath, std::ios::out | std::ios::trunc);
		if (!registerHeaderOss)
		{
			SUZZA_LOG_ERROR("打开文件失败! ", registerHeaderPath);
			return;
		}

		/* 生成头文件文本内容 */
		registerHeaderOss << GenerateRegisterHeaderFileStr() << std::endl;
		registerHeaderOss.close();

		std::string registerSourcePath = m_cacheOutFilePath + "/RegisterAllClass.cpp";
		std::ofstream registerSourceOss(registerSourcePath, std::ios::out | std::ios::trunc);
		if (!registerSourceOss)
		{
			SUZZA_LOG_ERROR("打开文件失败! ", registerSourcePath);
			return;
		}

		registerSourceOss << GenerateRegisterSourceFileHeaderStr() << std::endl;

		for (auto& pair : m_classInfo)
		{
			WriteHeaderFile(pair);
			WriteSourceFile(pair);
			//SUZZA_LOG_INFO("ClassName: ", pair.first, " Macro defined line: ", pair.second.macroDefinedLine);

			registerSourceOss << GenerateRegisterSourceFileIncludeStr(pair);
		}

		registerSourceOss << GenerateRegisterSourceFileFunctionBeginStr();

		for (auto& pair : m_classInfo)
		{
			registerSourceOss << GenerateRegisterSourceFileFunctionBodyStr(pair);
		}

		registerSourceOss << GenerateRegisterSourceFileFunctionEndStr();

		registerSourceOss.close();
	}

	void ReflectFileWriter::SetOutFilePath(const std::string& OutFilePath)
	{
		m_cacheOutFilePath = OutFilePath;
	}

	void ReflectFileWriter::SetProjectPath(const std::string& ProjectPath)
	{
		m_cacheProjectPath = ProjectPath;
	}

	void ReflectFileWriter::SetClassInfo(const std::unordered_map<std::string, ClassInfo>& ClassInfoData)
	{
		m_classInfo = ClassInfoData;
	}

	void ReflectFileWriter::WriteHeaderFile(const std::pair<std::string, ClassInfo>& ClassData)
	{
		bool previousParseSuccess = true;
		if (!ClassData.second.IsValid())
		{
			previousParseSuccess = false;
		}

		/* 拼接得到最终文件要写出的路径 */
		usize classNameIndex = ClassData.first.find("::");

		std::string fileName{};
		std::string className{};
		if (classNameIndex != std::string::npos)
		{
			className = ClassData.first.substr(classNameIndex + 2);
			fileName = className + ".generated.h";
		}
		else
		{
			className = ClassData.first;
			fileName = className + ".generated.h";
		}

		std::string fileFinalPath = m_cacheOutFilePath + "/" + fileName;

		/* 正式生成反射文件 */
		std::ofstream oss(fileFinalPath, std::ios::out | std::ios::trunc);
		if (!oss)
		{
			SUZZA_LOG_ERROR("打开文件失败! ", fileFinalPath);
			return;
		}

		/* 如果先前的解析失败, 对每个类创建空反射代码文件 */
		if (!previousParseSuccess)
		{
			oss.close();
			return;
		}

		oss << GenerateReflectString::GenerateClassHeaderString(ClassData, className);

		oss.close();
	}

	void ReflectFileWriter::WriteSourceFile(const std::pair<std::string, ClassInfo>& ClassData)
	{
		bool previousParseSuccess = true;
		if (!ClassData.second.IsValid())
		{
			previousParseSuccess = false;
		}

		/* 拼接得到最终文件要写出的路径 */
		usize classNameIndex = ClassData.first.find("::");

		std::string fileName{};
		std::string className{};
		if (classNameIndex != std::string::npos)
		{
			className = ClassData.first.substr(classNameIndex + 2);
			fileName = className + ".generated.cpp";
		}
		else
		{
			className = ClassData.first;
			fileName = className + ".generated.cpp";
		}
		
		std::string fileFinalPath = m_cacheOutFilePath + "/" + fileName;

		/* 获取被反射文件的相对路径 */
		std::filesystem::path classPath = std::filesystem::canonical(ClassData.second.classPath);
		std::filesystem::path projectPath = std::filesystem::canonical(m_cacheProjectPath);

		std::string includeClassPath = std::filesystem::relative(classPath, projectPath).generic_string();

		/* 正式生成反射文件 */
		std::ofstream oss(fileFinalPath, std::ios::out | std::ios::trunc);
		if (!oss)
		{
			SUZZA_LOG_ERROR("打开文件失败! ", fileFinalPath);
			return;
		}

		/* 如果先前的解析失败, 对每个类创建空反射代码文件 */
		if (!previousParseSuccess)
		{
			oss.close();
			return;
		}

		oss << GenerateReflectString::GenerateClassSourceString(ClassData, includeClassPath, className, classPath.generic_string());

		oss.close();
	}

	std::string ReflectFileWriter::GenerateRegisterHeaderFileStr()
	{
		std::stringstream stream;

		stream << "#pragma once\n" << std::endl;
		stream << R"(#include "macros/ReflectMacros.h")" << "\n" << std::endl;
		stream << R"(#undef SUZZA_REGISTER_ALL_REFLECT_CLASS)" << std::endl;
		stream << R"(#define SUZZA_REGISTER_ALL_REFLECT_CLASS suz::RegisterAllClass::RegisterReflectClass();)" << "\n" << std::endl;

		stream << R"(namespace suz)" << "\n{\n	" << "class RegisterAllClass\n	{\n	" << "public:\n		";
		stream<< R"(static void RegisterReflectClass();)" << "\n	};\n}" << std::endl;

		return stream.str();
	}

#pragma region GenericSource
	std::string ReflectFileWriter::GenerateRegisterSourceFileHeaderStr()
	{
		std::stringstream stream;

		stream << R"(#include "RegisterAllClass.h")" << std::endl;
		stream << R"(#include "Engine.h")" << std::endl;
		stream << R"(#include "ReflectionInfo.h")" << std::endl;

		return stream.str();
	}

	std::string ReflectFileWriter::GenerateRegisterSourceFileFunctionBeginStr()
	{
		std::stringstream stream;

		stream << "\n" << "namespace suz\n{\n	void RegisterAllClass::RegisterReflectClass()\n	{		" << std::endl;
		stream << R"(		ReflectionSystem& reflectSystem = Engine::GetEngineInstance().GetReflectionSystem();)" << "\n" << std::endl;

		return stream.str();
	}

	std::string ReflectFileWriter::GenerateRegisterSourceFileFunctionEndStr()
	{
		std::stringstream stream;

		stream << "	}\n}";

		return stream.str();
	}

	std::string ReflectFileWriter::GenerateRegisterSourceFileIncludeStr(const std::pair<std::string, ClassInfo>& ClassData)
	{
		if (!ClassData.second.IsValid())
		{
			return std::string();
		}

		std::string filePath{};

		usize classNameStartIndex = ClassData.first.rfind("::");
		if (classNameStartIndex != std::string::npos)
		{
			filePath = m_cacheProjectPath + ClassData.first.substr(classNameStartIndex + 2) + ".h";
		}
		else
		{
			filePath = m_cacheProjectPath + ClassData.first;
		}

		std::filesystem::path classPath = std::filesystem::canonical(ClassData.second.classPath);
		std::filesystem::path projectPath = std::filesystem::canonical(m_cacheProjectPath);

		std::string relativeFilePath = std::filesystem::relative(classPath, projectPath).generic_string();

		std::stringstream stream;

		stream << R"(#include ")" << relativeFilePath << "\"" << std::endl;

		return stream.str();
	}

	std::string ReflectFileWriter::GenerateRegisterSourceFileFunctionBodyStr(const std::pair<std::string, ClassInfo>& ClassData)
	{
		if (!ClassData.second.IsValid())
		{
			return std::string();
		}

		std::string className{};

		usize classNameStartIndex = ClassData.first.rfind("::");
		if (classNameStartIndex != std::string::npos)
		{
			className = ClassData.first.substr(classNameStartIndex + 2);
		}
		else
		{
			className = ClassData.first;
		}

		std::stringstream stream;

		stream << R"(		const ClassInfo& )" << className << "ClassInfo = " << className << "::StaticClass();" << std::endl;
		stream << R"(		reflectSystem.RegisterClassInfo(")" << className << R"(", )" << className << "ClassInfo);\n" << std::endl;

		return stream.str();
	}

#pragma endregion
}