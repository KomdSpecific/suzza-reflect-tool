#pragma once

#include "struct/ReflectStructInfo.h"

#include <clang/AST/DeclCXX.h>

namespace suz
{
	struct FunctionAnnoInfo
	{
		std::string functionName{};
		std::string fieldName{};
		EFunctionType functionType = EFunctionType::Unknown;
	};

	class MethodFilterLibrary
	{
	public:
		static void CollectGetterSetterMethod(
			clang::CXXRecordDecl* RecordDecl,
			ClassInfo& OutClassInfo
		);

	private:
		static void CheckIsTargetMethod(
			clang::CXXRecordDecl* RecordDecl,
			std::vector<FieldInfo>& OutFieldInfos
		);

		static FunctionAnnoInfo ReadAnnotationArgs(const clang::Attr* Attr, const clang::CXXMethodDecl* MethodDecl);
	};
}