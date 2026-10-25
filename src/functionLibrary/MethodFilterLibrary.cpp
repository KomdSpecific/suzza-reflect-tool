#include "MethodFilterLibrary.h"
#include "EnumParseLibrary.h"

#include <clang/AST/Attr.h>
#include <unordered_map>
#include <array>

#include "debug/SuzzaLog.h"

namespace suz
{
	void MethodFilterLibrary::CollectGetterSetterMethod(
		clang::CXXRecordDecl* RecordDecl,
		const std::string& GetterPrefix,
		const std::string& SetterPrefix,
		ClassInfo& OutClassInfo
	)
	{
		if (GetterPrefix.empty() || SetterPrefix.empty()) return;

		CheckIsTargetMethod(RecordDecl, GetterPrefix, SetterPrefix, OutClassInfo.fields);
	}

	void MethodFilterLibrary::CheckIsTargetMethod(
		clang::CXXRecordDecl* RecordDecl,
		const std::string& GetterPrefix,
		const std::string& SetterPrefix,
		std::vector<FieldInfo>& OutFieldInfos
	)
	{
		std::unordered_map<std::string, std::array<std::string, 2>> GetSetFuncMap{};

		for (clang::CXXMethodDecl* methodDecl : RecordDecl->methods())
		{
			/* 遍历所有的方法注解 */
			for (const clang::Attr* attr : methodDecl->attrs())
			{
				/* 收集函数的信息 */
				FunctionAnnoInfo annoInfo = ReadAnnotationArgs(attr, methodDecl);

				if (annoInfo.functionType == EFunctionType::GetterFunc)
				{
					GetSetFuncMap[annoInfo.fieldName][0] = annoInfo.functionName;
				}
				else if (annoInfo.functionType == EFunctionType::SetterFunc)
				{
					GetSetFuncMap[annoInfo.fieldName][1] = annoInfo.functionName;
				}
			}
		}

		for (FieldInfo& field : OutFieldInfos)
		{
			auto iterator = GetSetFuncMap.find(field.name);

			if (iterator != GetSetFuncMap.end() && !iterator->second[0].empty())
			{
				field.hasGetterMethod = true;
				field.getterMethodName = iterator->second[0];
				//SUZZA_LOG_INFO("FieldName: ", field.name, ",  GetterName: ", field.getterMethodName);
			}

			if (iterator != GetSetFuncMap.end() && !iterator->second[1].empty())
			{
				field.hasSetterMethod = true;
				field.setterMethodName = iterator->second[1];
				//SUZZA_LOG_INFO("FieldName: ", field.name, ",  SetterName: ", field.setterMethodName);
			}
		}
	}

	FunctionAnnoInfo MethodFilterLibrary::ReadAnnotationArgs(const clang::Attr* Attr, const clang::CXXMethodDecl* MethodDecl)
	{
		const clang::AnnotateAttr* annotateAttr = clang::dyn_cast<clang::AnnotateAttr>(Attr);
		if (!annotateAttr) return {};

		FunctionAnnoInfo annoInfo{};

		/* 遍历注解的每一个参数 */
		auto argsRange = annotateAttr->args();
		for (auto iterator = annotateAttr->args_begin(); iterator != annotateAttr->args_end(); iterator++)
		{
			const clang::StringLiteral* ExprStr = clang::dyn_cast<clang::StringLiteral>((*iterator)->IgnoreParenImpCasts());
			if (!ExprStr) continue;

			llvm::StringRef argStr = ExprStr->getString();

			/* 第一个参数永远固定是字段名 */
			if (iterator == annotateAttr->args_begin())
			{
				annoInfo.fieldName = argStr.str();
				continue;
			}

			usize enumSeparate = argStr.find("::");
			if (enumSeparate == llvm::StringRef::npos) continue;

			llvm::StringRef enumName = argStr.substr(0, enumSeparate);
			llvm::StringRef enumValue = argStr.substr(enumSeparate + 2);

			if (enumName == "EFunctionType")
			{
				annoInfo.functionType = EnumParseLibrary::GainEnumInfo(enumValue);
				annoInfo.functionName = MethodDecl->getNameAsString();
			}
		}

		return annoInfo;
	}
}