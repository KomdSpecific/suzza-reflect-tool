#pragma once

#include "struct/ReflectStructInfo.h"

#include <string>

namespace suz
{
	class GenerateReflectString
	{
	public:
		static std::string GenerateClassHeaderString(
			const std::pair<std::string, ClassInfo>& ClassData,
			const std::string& ClassName
		);
		static std::string GenerateClassSourceString(
			const std::pair<std::string, ClassInfo>& ClassData,
			const std::string& IncludeClassPath,
			const std::string& ClassName,
			const std::string& ClassPath
		);

		static std::string GenerateContainerCode(const FieldInfo& InFieldInfo);
	};
}