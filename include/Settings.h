#pragma once

class Settings
{
public:
	static void Load();
	static bool CreateGameSetting(const std::string_view& a_Key, const std::string_view& a_Value);

	static inline float MinimumMinutesForDeletion = { 5.0 };

private:
	static inline std::filesystem::path TOML_PATH = { L"Data\\SKSE\\Plugins\\AdMortemPermadeath.toml" };
};