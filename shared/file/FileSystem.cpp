#include "FileSystem.h"
#include "SuzzaGenerateconfig.h"

#include <fstream>
#include <string>

#if defined WIN32
#include <Windows.h>
#elif defined (_APPLE_)
#include <mach-o/dyld.h>
#elif defined (_linux_)
#include <unistd.h>
#include <limits.h>
#endif

namespace suz
{
	std::filesystem::path FileSystem::GetExecutableFolder()
	{
#if defined WIN32
		wchar_t buf[MAX_PATH];
		GetModuleFileNameW(NULL, buf, MAX_PATH);
		return std::filesystem::path(buf).remove_filename();

#elif defined (_APPLE_)
		uint32 size = 0;
		_NSGetExecutablePath(nullptr, &size);
		std::string tmp(size, '\0');
		_NSGetExecutablePath(tmp.data(), &size);
		return std::filesystem::weakly_canonical(std::filesystem::path(tmp)).remove_filename();

#elif defined (_linux_)
		return std::filesystem::weakly_canonical(std::filesystem::read_symlink("/proc/self/exe")).remove_filename();

#else
		return std::filesystem::current_path();

#endif
	}

	std::filesystem::path FileSystem::GetSourceFolder()
	{
#if defined TARGET_SCAN_HEADER_ROOT
		std::filesystem::path path = std::filesystem::path(TARGET_SCAN_HEADER_ROOT);
		if (std::filesystem::exists(path))
		{
			return path;
		}
#endif
		return std::filesystem::path();
	}

	std::filesystem::path FileSystem::GetShadersFolder()
	{
#if defined (SHADERS_ROOT)
		std::filesystem::path path = std::filesystem::path(SHADERS_ROOT);
		if (std::filesystem::exists(path))
		{
			return path;
		}

#endif

		/* 直接通过项目执行路径读取 */
		return std::filesystem::weakly_canonical(GetExecutableFolder() / "shaders");
	}

	std::filesystem::path FileSystem::GetAssetsFolder()
	{
#if defined (ASSETS_ROOT)
		std::filesystem::path path = std::filesystem::path(ASSETS_ROOT);
		if (std::filesystem::exists(path))
		{
			return path;
		}

#endif

		return std::filesystem::weakly_canonical(GetExecutableFolder() / "assets");
	}
}