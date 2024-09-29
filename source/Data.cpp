#include "Data.h"

#include "REX/W32/OLE32.h"
#include "REX/W32/SHELL32.h"

auto Data::GetSingleton() -> Data*
{
    static Data singleton;

    return std::addressof(singleton);
}

auto Data::GetSaveFileDirectory() -> std::optional<std::filesystem::path>
{
    if (!save_files) {
        const auto ini_settings = RE::INISettingCollection::GetSingleton();

        if (!ini_settings) {
            return std::nullopt;
        }

        wchar_t* buffer = nullptr;
        const auto result = REX::W32::SHGetKnownFolderPath(REX::W32::FOLDERID_Documents, REX::W32::KF_FLAG_DEFAULT, nullptr, std::addressof(buffer));

        std::unique_ptr<wchar_t[], decltype(&REX::W32::CoTaskMemFree)> path_ptr(buffer, REX::W32::CoTaskMemFree);

        if (!path_ptr || result != 0) {
            return std::nullopt;
        }

        std::filesystem::path path = path_ptr.get();

        path /= "My Games"sv;
        path /= std::filesystem::exists("steam_api64.dll") ? "Skyrim Special Edition" : "Skyrim Special Edition GOG";
        path /= ini_settings->GetSetting("sLocalSavePath:General")->GetString();

        save_files = path;

        return path;
    }

    return save_files;
}

class MessageCallback : public RE::IMessageBoxCallback
{
public:
    MessageCallback() = default;
    ~MessageCallback() override = default;

    void Run(RE::IMessageBoxCallback::Message a_message) override
    {
        const auto response = static_cast<std::int32_t>(a_message) - 4;

        switch (response) {
        case 0:
            RE::Main::GetSingleton()->quitGame = true;
            break;
        case 1:
            Data::QuitToMainMenu();
            break;
        default:
            break;
        }
    }
};

void Data::QueueMessage(const std::string_view& a_source, const std::string_view& a_target, const std::string_view& a_location, const std::string_view& a_race, const std::uint16_t a_level, const float a_days)
{
    const auto factory_manager = RE::MessageDataFactoryManager::GetSingleton();
    const auto interface_strings = RE::InterfaceStrings::GetSingleton();

    if (!factory_manager || !interface_strings) {
        return;
    }

    const auto message_factory = factory_manager->GetCreator<RE::MessageBoxData>(interface_strings->messageBoxData);
    const auto message = message_factory ? message_factory->Create() : nullptr;

    if (!message) {
        return;
    }

    message->bodyText = std::vformat(Settings::Message, std::make_format_args(a_source, a_target, a_location, a_target, a_target, a_race, a_level, a_days, Settings::Tip));

    message->buttonText.push_back(Settings::QuitToDesktop.data());
    message->buttonText.push_back(Settings::QuitToMainMenu.data());

    message->unk38 = 10;
    message->callback = RE::BSTSmartPointer<RE::IMessageBoxCallback>{ new MessageCallback() };
    message->unk4C = 4;

    message->QueueMessage();
}