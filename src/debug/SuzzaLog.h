#pragma once

#include <string>
#include <type_traits>
#include <iostream>

#if PRODUCTION_BUILD == 0
#define SUZZA_LOG_INFO(...) suz::SuzzaLogger::SuzLogInfo(__VA_ARGS__)
#define SUZZA_LOG_WARN(...) suz::SuzzaLogger::SuzLogWarn(__VA_ARGS__)
#define SUZZA_LOG_ERROR(...) suz::SuzzaLogger::SuzLogError(__VA_ARGS__)

#else
#define SUZZA_LOG_INFO(...) ((void)0)
#define SUZZA_LOG_WARN(...) ((void)0)
#define SUZZA_LOG_ERROR(...) ((void)0)

#endif

namespace suz
{
	class SuzzaLogger
	{
	public:
		static void SuzLogInfo(const char* msg, ...);
		static void SuzLogWarn(const char* msg, ...);
		static void SuzLogError(const char* msg, ...);

		template<typename... Args>
		static void SuzLogInfo(Args&&...args)
		{
			std::cout << "[LOG INFO] ";
			((std::cout << std::forward<Args>(args)), ...);
			std::cout << "\n";
		}

		template<typename... Args>
		static void SuzLogWarn(Args&&...args)
		{
			std::cout << "[LOG WARN] ";
			((std::cout << std::forward<Args>(args)), ...);
			std::cout << "\n";
		}

		template<typename... Args>
		static void SuzLogError(Args&&...args)
		{
			std::cout << "[LOG ERROR] ";
			((std::cout << std::forward<Args>(args)), ...);
			std::cout << "\n";
		}
	};
}