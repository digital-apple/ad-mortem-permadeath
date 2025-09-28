#include "Settings.h"

void Settings::Load()
{
	CreateGameSetting("sAMP_Engraving", "Name: %s\nRace: %s\nLevel: %i\nIn-game days spent alive: %i\n\nCause of death: %s");
	CreateGameSetting("sAMP_Tip", "Tip: If you intend to keep playing, it is advisable to restart the game to avoid data corruption!");
	CreateGameSetting("sAMP_QuitToDesktop", "Quit to Desktop (Recommended)");
	CreateGameSetting("sAMP_QuitToMainMenu", "Quit to Main Menu");

	CreateGameSetting("sAMP_DefaultSourceName", "Godhead");
	CreateGameSetting("sAMP_DefaultPlayerName", "Player");

	CreateGameSetting("sAMP_Environment", "Environment");
	
	try {
		//const auto settings = toml::parse(TOML_PATH);




	} catch ([[maybe_unused]]const toml::syntax_error& error) {
		// SKSE::stl::report_and_error(std::format("Settings::Load ~ Failed to parse TOML! <{}>", error.what()));
	}
}

bool Settings::CreateGameSetting(const std::string_view& a_Key, const std::string_view& a_Value)
{
	INFO("Settings::CreateGameSetting >> Creating Key: {} with Value: {}", a_Key, a_Value);

	const auto game_settings = RE::GameSettingCollection::GetSingleton();

	if (!game_settings) {
		ERROR("Settings::CreateGameSetting >> Failed to obtain the GameSettingCollection singleton!");

		return false;
	}

	auto setting = RE::malloc<RE::Setting>();

	setting->name = const_cast<char*>(a_Key.data());
	setting->data.s = const_cast<char*>(a_Value.data());

	if (!SKSE::stl::emplace_vtable<RE::Setting>(setting)) {
		ERROR("Settings::CreateGameSetting >> Failed to create Key: {}", a_Key);

		return false;
	}

	game_settings->InsertSetting(setting);

	INFO("Settings::CreateGameSetting >> Finished creating Key: {}", a_Key);

	return true;
}

auto Settings::GetGameSetting(const std::string_view& a_Key) -> const char*
{
	const auto game_settings = RE::GameSettingCollection::GetSingleton();

	if (!a_Key.empty() && game_settings) {
		const auto setting = game_settings->GetSetting(a_Key.data());

		if (setting) {
			return setting->GetString();
		}
	}

	return "";
}