#include "SuzzaGenerateTool.h"

#include "debug/SuzzaLog.h"

/* 传入参数
	1. 文件输出目录
	2. 项目源码目录
	3. compile_commands.json路径
	*/
int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		SUZZA_LOG_ERROR("参数错误! 用法: ");
		SUZZA_LOG_ERROR(argv[0], "<输出反射文件路径>  <项目源码目录>  <compile_commands.json路径>");
		return EXIT_FAILURE;
	}

	std::string outReflectFile = argv[1];
	std::string projectPath = argv[2];
	std::string jsonPath = argv[3];

	suz::SuzzaGenerateTool tool;
	if (!tool.Init(outReflectFile, projectPath, jsonPath))
	{
		SUZZA_LOG_ERROR("Suzza Header Tool Init Failure!");
		return EXIT_FAILURE;
	}

	try
	{
		tool.Run();
		tool.WriteReflectFile();
	}
	catch (const std::exception& e)
	{
		SUZZA_LOG_ERROR("Suzza Header Tool Running Error: ", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}