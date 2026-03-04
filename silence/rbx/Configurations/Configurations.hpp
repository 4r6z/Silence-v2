#pragma once

#include <ShlObj.h>
#include <string>
#include <filesystem>
#include "../Headers/XorStr.hpp"

static std::string get_appdata_path()
{
	wchar_t path[MAX_PATH];
	SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path);
	std::wstring wstrPath(path);
	return std::string(wstrPath.begin(), wstrPath.end());
}

static std::string GetConfigFolderPath()
{
    std::string silenceFolderPath = get_appdata_path() + XorStr("\\silence");

    std::string silenceConfigsFolderPath = silenceFolderPath + XorStr("\\configs");

    if (!std::filesystem::exists(silenceConfigsFolderPath))
    {
        std::filesystem::create_directory(silenceConfigsFolderPath);
    }

    return silenceConfigsFolderPath;
}

class configs {
public:
	static void save(const char* name);
	static void load(const char* name);
    //static void getConfigPath(const char* name);
};