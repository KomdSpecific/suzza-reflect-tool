#include "EnumParseLibrary.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSwitch.h>

#include "debug/SuzzaLog.h"

namespace suz
{
	void EnumParseLibrary::ReadEnumStructInfo(const llvm::StringRef& EnumName, const llvm::StringRef& EnumValue, FieldInfo& OutFieldInfo)
	{
		if (EnumName == "Editability")
		{
			OutFieldInfo.editableEnumStr = EnumName.str() + "::" + EnumValue.str();
		}
		else if (EnumName == "EFunctionType")
		{

		}
	}

	EFunctionType EnumParseLibrary::GainEnumInfo(const llvm::StringRef& EnumValue)
	{
		return llvm::StringSwitch<EFunctionType>(EnumValue.trim())
			.Case("GetterFunc", EFunctionType::GetterFunc)
			.Case("SetterFunc", EFunctionType::SetterFunc)
			.Default(EFunctionType::Unknown);
	}
}