#pragma once

#include <filesystem>

namespace suz
{
	class FileSystem
	{
	public:
		static std::filesystem::path GetExecutableFolder();

		static std::filesystem::path GetSourceFolder();

		static std::filesystem::path GetShadersFolder();

		static std::filesystem::path GetAssetsFolder();
	};
}