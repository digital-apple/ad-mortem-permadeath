#include "System.h"

#include "Settings.h"

auto System::GetSingleton() -> System*
{
    static System singleton;

    return std::addressof(singleton);
}

bool System::IsDead(RE::Actor* a_Actor)
{
    // Check for Essential flag!

    if (a_Actor && a_Actor == RE::PlayerCharacter::GetSingleton()) {
        const auto av_owner = a_Actor ? a_Actor->AsActorValueOwner() : nullptr;

        if (av_owner) {
            const auto current_health = av_owner->GetActorValue(RE::ActorValue::kHealth);

            if (current_health < 0.f) {
                return true;
            }
        }
    }

    return false;
}

void System::Delete(RE::Actor* a_Target, const std::string_view& a_Source)
{
    INFO("System::Delete >> CALL!");

    const auto save_manager = RE::BGSSaveLoadManager::GetSingleton();

    if (!save_manager) {
        return;
    }

    if (!save_manager->PopulateSaveList()) {
        return;
    }

    const auto save_files = System::GetSingleton()->GetSaveFilesDirectory();

    if (!save_files) {
        return;
    }

    // Fix this shit

    const auto target_name = a_Target ? a_Target->GetName() : "Player";
    const auto source_name = a_Source.data();
    const auto race = a_Target ? a_Target->GetRace() ? a_Target->GetRace()->GetName() : "Nord" : "Nord";
    const auto level = a_Target ? a_Target->GetLevel() : std::uint16_t{ 0 };
    const auto location = a_Target->GetCurrentLocation() ? a_Target->GetCurrentLocation()->GetName() : a_Target->GetWorldspace() ? a_Target->GetWorldspace()->GetName() : "Tamriel";

    const auto calendar = RE::Calendar::GetSingleton();

    const auto days_passed = calendar ? std::floorf(calendar->GetDaysPassed()) : 0.f;

    QueueDeathMessage({ target_name, source_name, race, location, level, days_passed });
}

auto System::GetSaveFilesDirectory() -> std::optional<std::filesystem::path>
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
    enum class Message
    {
        kNone = -1,
        kQuitToDesktop = 0,
        kQuitToMainMenu = 1
    };

    MessageCallback() = default;
    ~MessageCallback() override = default;

    void Run(RE::IMessageBoxCallback::Message a_message) override
    {
        const auto response = static_cast<Message>(static_cast<std::int32_t>(a_message) - 4);

        switch (response) {
        case Message::kQuitToDesktop:
            RE::Main::GetSingleton()->quitGame = true;
            break;
        case Message::kQuitToMainMenu:
            Native::QuitToMainMenu();
            break;
        default:
            break;
        }
    }
};

void System::QueueDeathMessage(const Engraving& a_Engraving)
{
    INFO("System::QueueDeathMessage >> CALL!");

    const auto factory_manager = RE::MessageDataFactoryManager::GetSingleton();
    const auto interface_strings = RE::InterfaceStrings::GetSingleton();

    if (!factory_manager || !interface_strings) {
        ERROR("System::QueueDeathMessage >> Failed to obtain critical singletons!");

        return;
    }

    const auto message_factory = factory_manager->GetCreator<RE::MessageBoxData>(interface_strings->messageBoxData);
    const auto message = message_factory ? message_factory->Create() : nullptr;

    if (!message) {
        ERROR("System::QueueDeathMessage >> Failed to create message!");

        return;
    }

    const auto sAMP_Engraving = Settings::GetGameSetting("sAMP_Engraving");
    const auto sAMP_Tip = Settings::GetGameSetting("sAMP_Tip");
    const auto sAMP_QuitToDesktop = Settings::GetGameSetting("sAMP_QuitToDesktop");
    const auto sAMP_QuitToMainMenu = Settings::GetGameSetting("sAMP_QuitToMainMenu");

    INFO("{} : {} : {} : {} : {} : {}", a_Engraving.target, a_Engraving.source, a_Engraving.location, a_Engraving.race, a_Engraving.level, a_Engraving.days);

    char buffer[0x104];
    std::snprintf(buffer, sizeof(buffer), sAMP_Engraving, a_Engraving.target.c_str(), a_Engraving.race.c_str(), a_Engraving.level, a_Engraving.days, a_Engraving.source.c_str());

    message->bodyText = std::format("{}\n\n{}", buffer, sAMP_Tip);

    message->buttonText.push_back(sAMP_QuitToDesktop);
    message->buttonText.push_back(sAMP_QuitToMainMenu);

    message->type = 10;
    message->callback = RE::BSTSmartPointer<RE::IMessageBoxCallback>{ new MessageCallback() };
    message->optionIndexOffset = 4;

    message->QueueMessage();
}

bool System::SkipFile(const std::string_view& a_playtime)
{
    if (Settings::MinimumMinutesForDeletion == 0.f) {
        return false;
    }

    float hours, minutes, seconds;

    std::stringstream stream(a_playtime.data());
    std::string segment;

    std::getline(stream, segment, '.');
    hours = std::stof(segment);

    std::getline(stream, segment, '.');
    minutes = std::stof(segment);

    stream >> seconds;

    const auto playtime = (hours * 60.f) + minutes + (seconds / 60.f);

    return playtime <= Settings::MinimumMinutesForDeletion;
}
