#pragma once

class Data
{
public:
    static auto GetSingleton() -> Data*;

    auto GetSaveFileDirectory() -> std::optional<std::filesystem::path>;
    void QueueMessage(const std::string_view& a_source, const std::string_view& a_target, const std::string_view& a_location, const std::string_view& a_race, const std::uint32_t a_level, const float a_days);
    void WriteOutput(const std::string_view& a_source, const std::string_view& a_target, const std::string_view& a_location, const std::string_view& a_race, const std::uint32_t a_level, const float a_days);
    bool SkipFile(const std::string_view& a_playtime);

    static bool QuitToMainMenu()
    {
        using func_t = decltype(&QuitToMainMenu);
        REL::Relocation<func_t> func{ RELOCATION_ID(54857, 55490) };
        return func();
    }

private:
    Data() = default;
    Data(const Data&) = delete;
    Data(Data&&) = delete;

    ~Data() = default;

    Data& operator=(const Data&) = delete;
    Data& operator=(Data&&) = delete;

    std::optional<std::filesystem::path> save_files;
};

struct Settings
{
    static inline void Load()
    {
        logger::info("Loading Settings!");

        const auto path = L"Data/SKSE/Plugins/ad-mortem-permadeath.ini";

        CSimpleIniA ini;

        ini.SetUnicode();

        ini.LoadFile(path);

        ReadSetting(ini, "Settings", "fMinimumMinutesForDeletion", MinimumMinutes);

        ReadSetting(ini, "Translations", "sMessage", Message);
        ReadSetting(ini, "Translations", "sTip", Tip);
        ReadSetting(ini, "Translations", "sQuitToDesktop", QuitToDesktop);
        ReadSetting(ini, "Translations", "sQuitToMainMenu", QuitToMainMenu);
    }

    static inline void ReadSetting(CSimpleIni& a_ini, const char* a_section, const char* a_key, std::string& a_setting)
    {
        auto found = a_ini.GetValue(a_section, a_key);

        if (found)
        {
            std::string result = found;
            std::size_t position = 0;

            while ((position = result.find("\\n", position)) != std::string::npos) {
                result.replace(position, 2, "\n");
                position += 1;
            }

            a_setting = result;
        }
    }

    static inline void ReadSetting(CSimpleIni& a_ini, const char* a_section, const char* a_key, float& a_setting)
    {
        auto found = a_ini.GetValue(a_section, a_key);

        if (found)
        {
            auto result = static_cast<float>(a_ini.GetDoubleValue(a_section, a_key));

            a_setting = result > 20.f ? 20.f : result;
        }
    }

    static inline float MinimumMinutes = 5.f;

    static inline std::string Message = "{} put an end to {}'s misery at {}!\n\n{}'s inscription:\n\nName: {}\nRace: {}\nLevel: {}\nIn-game days spent alive: {}\n\n{}";
    static inline std::string Tip = "Tip: If you intend to keep playing, it is advisable to restart the game to avoid data corruption!";
    static inline std::string QuitToDesktop = "Quit to Desktop (Recommended)";
    static inline std::string QuitToMainMenu = "Quit to Main Menu";
};
