#include "SuzzaLog.h"

#include <cstdio>
#include <cstdarg>

namespace suz
{
	void SuzzaLogger::SuzLogInfo(const char* msg, ...)
	{
		va_list valueList{};

		/* 告诉系统函数可变参数在msg后面 */
		va_start(valueList, msg);
		printf("[LOG INFO] ");
		vprintf(msg, valueList);
		printf("\n");
		va_end(valueList);
	}

	void suz::SuzzaLogger::SuzLogWarn(const char* msg, ...)
	{
		va_list valueList{};

		/* 告诉系统函数可变参数在msg后面 */
		va_start(valueList, msg);
		printf("[LOG WARN] ");
		vprintf(msg, valueList);
		printf("\n");
		va_end(valueList);
	}

	void suz::SuzzaLogger::SuzLogError(const char* msg, ...)
	{
		va_list valueList{};

		/* 告诉系统函数可变参数在msg后面 */
		va_start(valueList, msg);
		printf("[LOG ERROR] ");
		vprintf(msg, valueList);
		printf("\n");
		va_end(valueList);
	}
}