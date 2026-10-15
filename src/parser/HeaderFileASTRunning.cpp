#include "HeaderFileASTRunning.h"
#include "HeaderFileASTVisitor.h"
#include "file/FileSystem.h"
#include "HeaderCollectPPCallback.h"

#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Frontend/CompilerInstance.h>
#include <unordered_set>

#include "SuzzaGenerateconfig.h"
#include "debug/SuzzaLog.h"

namespace suz
{
	void ConstructClangArgs(const std::vector<std::string>& SourceArgs, std::vector<std::string>& OutArgs);

	class HeaderFileASTConsumer : public clang::ASTConsumer
	{
	public:
		HeaderFileASTConsumer(
			std::unordered_map<std::string, std::vector<clang::SourceLocation>>* MacroExpandMap,
			std::unordered_map<std::string, ClassInfo>* ClassInfoList
		)
			: m_macroExpandList(MacroExpandMap), m_classInfoList(ClassInfoList)
		{
		}

		void HandleTranslationUnit(clang::ASTContext& ASTContext) override
		{
			HeaderFileASTVisitor visitor(*m_macroExpandList, *m_classInfoList);
			visitor.TraverseDecl(ASTContext.getTranslationUnitDecl());
		}

		std::unordered_map<std::string, std::vector<clang::SourceLocation>>* m_macroExpandList;
		std::unordered_map<std::string, ClassInfo>* m_classInfoList;
	};

	class HeaderFileASTFrontendAction : public clang::ASTFrontendAction
	{
	public:
		HeaderFileASTFrontendAction(
			std::unordered_map<std::string, std::vector<clang::SourceLocation>>* MacroExpandMap,
			std::unordered_map<std::string, ClassInfo>* ClassInfoList
		)
			: m_macroExpandList(MacroExpandMap), m_classInfoList(ClassInfoList)
		{
		}

		/* 源文件开始处理时, 注册预处理器回调 */
		bool BeginSourceFileAction(clang::CompilerInstance& CI) override
		{
			CI.getPreprocessor().addPPCallbacks(
				std::make_unique<HeaderCollectPPCallback>(*m_macroExpandList, CI.getSourceManager())
			);
			return true;
		}

		/* 空ASTConsumer, 不解析AST, 只预处理 */
		std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& CI, llvm::StringRef) override
		{
			return std::make_unique<HeaderFileASTConsumer>(m_macroExpandList, m_classInfoList);
		}

		std::unordered_map<std::string, std::vector<clang::SourceLocation>>* m_macroExpandList;
		std::unordered_map<std::string, ClassInfo>* m_classInfoList;
	};

	class HeaderFieldCollectActionFactory : public clang::tooling::FrontendActionFactory
	{
	public:
		HeaderFieldCollectActionFactory(
			std::unordered_map<std::string, std::vector<clang::SourceLocation>>* MacroExpandMap,
			std::unordered_map<std::string, ClassInfo>* ClassInfoList
		)
			: m_macroExpandList(MacroExpandMap), m_classInfoList(ClassInfoList)
		{
		}

		std::unique_ptr<clang::FrontendAction> create() override
		{
			return std::make_unique<HeaderFileASTFrontendAction>(m_macroExpandList, m_classInfoList);
		}

		std::unordered_map<std::string, std::vector<clang::SourceLocation>>* m_macroExpandList;
		std::unordered_map<std::string, ClassInfo>* m_classInfoList;
	};

	bool RunToolOnParseHeaders(
		const std::string& FilePath,
		const std::vector<std::string>& CompileArgs,
		std::unordered_map<std::string, std::vector<clang::SourceLocation>>& OutMacroExpandMap,
		std::unordered_map<std::string, ClassInfo>& OutClassInfo
	)
	{
		if (FilePath.empty() || CompileArgs.empty())
		{
			SUZZA_LOG_ERROR("没有可以解析的头文件和有效参数!");
			return false;
		}

		std::vector<std::string> finalArgs{};
		ConstructClangArgs(CompileArgs, finalArgs);
		clang::tooling::FixedCompilationDatabase database(FileSystem::GetExecutableFolder().string(), finalArgs);

		std::unique_ptr<HeaderFieldCollectActionFactory> factory = std::make_unique<HeaderFieldCollectActionFactory>(&OutMacroExpandMap, &OutClassInfo);
		/* 将头文件列表传给ClangTool */
		clang::tooling::ClangTool tool(database, FilePath);
		/* 对每一个列表中的头文件进行解析 */
		int ret = tool.run(factory.get());

		return ret == 0;
	}

	void ConstructClangArgs(const std::vector<std::string>& SourceArgs, std::vector<std::string>& OutArgs)
	{
		if (SourceArgs.empty()) return;

		/*for (const auto& s : SourceArgs)
		{
			SUZZA_LOG_INFO(s);
		}*/

		std::unordered_set<std::string> blackList = {
			"/nologo", "/TP", "/TC", "/GR", "/Zi", "/RTC1", "/FS",
			"/MD", "/MDd", "/MT", "/MTd", "/O", "/O1", "/O2", "/Od", "/Ob",
			"/W", "/W0", "/W1", "/W2", "/W3", "/W4", "/WX", "/EH", "/EHsc", "/EHs", "/EHa",
			"/Fe", "/Fo", "/c", "-external:W0"
		};

		/* 跳过cl.exe程序名 (第一个参数) */
		for (usize argIndex = 1; argIndex < SourceArgs.size(); argIndex++)
		{
			const std::string& arg = SourceArgs[argIndex];
			llvm::StringRef ref(arg);

			/* 1. 跳过MSVC专属参数, clang不识别, 直接丢弃 */
			if (blackList.contains(arg))
			{
				continue;
			}

			/* 2. -I 头文件路径 */
			if (ref.startswith("-I"))
			{
				llvm::StringRef path = ref.substr(2).trim();
				if (!path.empty())
				{
					OutArgs.push_back("-I");
					OutArgs.push_back(path.str());
				}
				continue;
			}

			/* 3. -D 宏定义路径 */
			if (ref.startswith("-D"))
			{
				llvm::StringRef macro = ref.substr(2).trim();
				if (!macro.empty())
				{
					OutArgs.push_back("-D");
					OutArgs.push_back(macro.str());
				}
				continue;
			}

			if (ref.startswith("-external:I"))
			{
				llvm::StringRef path = ref.substr(11).trim();
				if (!path.empty())
				{
					OutArgs.push_back("-I");
					OutArgs.push_back(path.str());
				}
				continue;
			}

			/* 4. 处理 */
			if (ref.startswith("-std:c++"))
			{
				llvm::StringRef standard = ref.substr(8).trim();
				if (!standard.empty())
				{
					std::string cxxStandard = "-std=c++" + standard.str();
					OutArgs.push_back(cxxStandard);
				}
				continue;
			}

			/* . 过滤其他以 "/" 开头的MSVC参数, 防止clang当成文件 */
			if (ref.startswith("/"))
			{
				continue;
			}
		}

		OutArgs.push_back("-I");
		OutArgs.push_back(LLVM_INCLUDE_ROOT);

		/* 关闭LLVM警告 */
#if PRODUCTION_BUILD == 1
		OutArgs.push_back("-w");
#endif
	}
}