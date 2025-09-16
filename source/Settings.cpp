#include "Settings.h"

void Settings::Load()
{
	CreateGameSetting("sAMP_Character", "Name: %s\nRace: %s\nLevel: %i\nIn-game days spent alive: %i");
	CreateGameSetting("sAMP_Tip", "Tip: If you intend to keep playing, it is advisable to restart the game to avoid data corruption!");
	CreateGameSetting("sAMP_QuitToDesktop", "Quit to Desktop (Recommended)");
	CreateGameSetting("sAMP_QuitToMainMenu", "Quit to Main Menu");

	CreateGameSetting("sAMP_DeathUnknown", "Unknown");
	CreateGameSetting("sAMP_DeathDrowning", "Drowning");
	CreateGameSetting("sAMP_DeathFalling", "Falling");
	CreateGameSetting("sAMP_DeathMelee", "Melee");
	CreateGameSetting("sAMP_DeathRanged", "Ranged");
	CreateGameSetting("sAMP_DeathMagic", "Magic");
	CreateGameSetting("sAMP_DeathEnvironmentalPhysical", "Environmental Physical");
	CreateGameSetting("sAMP_DeathEnvironmentalMagical", "Environmental Magical");
	
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