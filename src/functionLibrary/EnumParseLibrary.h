#pragma once

#include "struct/ReflectStructInfo.h"

#include <string>
#include <vector>

namespace llvm
{
	class StringRef;
}

namespace suz
{
	class EnumParseLibrary
	{
	public:
		static void ReadEnumStructInfo(const llvm::StringRef& EnumName, const llvm::StringRef& EnumValue, FieldInfo& OutFieldInfo);

		static EFunctionType GainEnumInfo(const llvm::StringRef& EnumValue);
	};
}