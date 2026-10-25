#include "SuzzaGenerateTool.h"
#include "parser/HeaderFileASTVisitor.h"
#include "parser/HeaderFileASTRunning.h"
#include "json/CompileCommandsJsonParser.h"
#include "struct/ReflectStructInfo.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	bool SuzzaGenerateTool::Init(const std::string& OutFilePath, const std::string& ProjectPath, const std::string& ParseJsonPath)
	{
		m_reflectFileWriter->SetOutFilePath(OutFilePath);
		m_reflectFileWriter->SetProjectPath(ProjectPath);

		llvm::json::Array parseArray = CompileCommandsJsonParser::ParseCompileCommandsJson(ParseJsonPath);

		return CollectHeaderFilePath(parseArray);
	}

	void SuzzaGenerateTool::Run()
	{
		std::unordered_map<std::string, ClassInfo> classInfo{};
		std::unordered_map<std::string, std::vector<clang::SourceLocation>> macroExpandList{};
		std::unordered_map<std::string, unsigned int> macroExpandLine{};

		usize beforeSizeInfo = classInfo.size();
		for (const llvm::json::Value& value : m_parseFileArr)
		{
			const llvm::json::Object* obj = value.getAsObject();

			llvm::StringRef file = obj->getString("file").value();
			llvm::StringRef command = obj->getString("command").value();
			std::vector<std::string> argList = CompileCommandsJsonParser::SplitCommandLine(command);

			bool success = RunToolOnParseHeaders(
				file.str(),
				argList,
				macroExpandList,
				macroExpandLine,
				classInfo
			);
			if (!success)
			{
				/* 没有插入任何数据, 解析失败! */
				if (classInfo.size() == beforeSizeInfo)
				{
					SUZZA_LOG_ERROR("解析头文件失败! ", file.str());
					continue;
				}
			}

			beforeSizeInfo = classInfo.size();
		}

		/*for (const auto& s : classInfo)
		{
			SUZZA_LOG_INFO("Class name: ", s.first);
			for (const auto& field : s.second.fields)
			{
				SUZZA_LOG_INFO("Field name: ", field.name, ",  Type name: ", field.typeName);

				if (field.hasGetterMethod)
				{
					SUZZA_LOG_INFO("GetterFunc: ", field.getterMethodName, " SetterFunc: ", field.setterMethodName);
					SUZZA_LOG_INFO();
				}
			}
			SUZZA_LOG_INFO("\n\n");
		}*/

		/*for (const auto& s : macroExpandLine)
		{
			SUZZA_LOG_INFO("\n");
			SUZZA_LOG_INFO("File path: ", s.first);
			SUZZA_LOG_INFO("Line: ", s.second);
		}*/

		if (!classInfo.empty())
		{
			m_reflectFileWriter->SetClassInfo(classInfo);
		}
	}

	void SuzzaGenerateTool::WriteReflectFile()
	{
		m_reflectFileWriter->WriteReflectSourceFile();
	}

	bool SuzzaGenerateTool::CollectHeaderFilePath(const llvm::json::Array& ParseArray)
	{
		if (ParseArray.empty())
		{
			SUZZA_LOG_ERROR("文件解析结果为空, 暂停解析!");
			return false;
		}

		for (const llvm::json::Value& value : ParseArray)
		{
			const llvm::json::Object* obj = value.getAsObject();
			if (!obj) continue;

			/* 对指定文件夹过滤 */
			llvm::StringRef file = obj->getString("file").value();
			if (file.contains("scene"))
			{
				m_parseFileArr.insert(m_parseFileArr.end(), value);
			}
		}

		return true;
	}
}