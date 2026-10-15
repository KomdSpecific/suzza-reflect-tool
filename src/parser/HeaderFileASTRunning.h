#pragma once

#include "struct/ReflectStructInfo.h"

#include <clang/Basic/SourceLocation.h>

namespace suz
{
	bool RunToolOnParseHeaders(
		const std::string& FilePath,
		const std::vector<std::string>& CompileArgs,
		std::unordered_map<std::string, std::vector<clang::SourceLocation>>& OutMacroExpandMap,
		std::unordered_map<std::string, ClassInfo>& OutClassInfo
	);
}