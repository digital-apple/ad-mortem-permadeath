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

    if (a_Actor == RE::PlayerCharacter::GetSingleton()) {
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

void System::Delete(DeathType a_DeathType, RE::Actor* a_SourceActor, RE::Actor* a_TargetActor)
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

    const auto source_name = a_SourceActor ? a_SourceActor->GetName() : "Godhead";
    const auto target_name = a_TargetActor ? a_TargetActor->GetName() : "Player";
    const auto race = a_TargetActor ? a_TargetActor->GetRace() ? a_TargetActor->GetRace()->GetName() : "Nord" : "Nord";
    const auto level = a_TargetActor->GetLevel();
    const auto location = a_TargetActor->GetCurrentLocation() ? a_TargetActor->GetCurrentLocation()->GetName() : a_TargetActor->GetWorldspace() ? a_TargetActor->GetWorldspace()->GetName() : "Tamriel";

    const auto calendar = RE::Calendar::GetSingleton();

    const auto days_passed = calendar ? std::floorf(calendar->GetDaysPassed()) : 0.f;

    QueueDeathMessage({ source_name, target_name, race, location, level, days_passed, a_DeathType });
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
    const auto game_settings = RE::GameSettingCollection::GetSingleton();

    if (!factory_manager || !interface_strings || !game_settings) {
        ERROR("System::QueueDeathMessage >> Failed to obtain critical singletons!");

        return;
    }

    const auto message_factory = factory_manager->GetCreator<RE::MessageBoxData>(interface_strings->messageBoxData);
    const auto message = message_factory ? message_factory->Create() : nullptr;

    if (!message) {
        ERROR("System::QueueDeathMessage >> Failed to create message!");

        return;
    }

    const auto sAMP_Character = game_settings->GetSetting("sAMP_Character");
    const auto sAMP_Tip = game_settings->GetSetting("sAMP_Tip");
    const auto sAMP_QuitToDesktop = game_settings->GetSetting("sAMP_QuitToDesktop");
    const auto sAMP_QuitToMainMenu = game_settings->GetSetting("sAMP_QuitToMainMenu");

    INFO("{} : {} : {} : {} : {} : {} : {}", a_Engraving.source, a_Engraving.target, a_Engraving.location, a_Engraving.race, a_Engraving.level, a_Engraving.days, std::to_underlying(a_Engraving.cause));

    char buffer[0x104];
    std::snprintf(buffer, sizeof(buffer), sAMP_Character->GetString(), a_Engraving.target.c_str(), a_Engraving.race.c_str(), a_Engraving.level, a_Engraving.days);

    message->bodyText = std::format("{}\n\nCause of Death: {}\n\n{}", buffer, TypeToString(a_Engraving.cause), sAMP_Tip->GetString());

    message->buttonText.push_back(sAMP_QuitToDesktop->GetString());
    message->buttonText.push_back(sAMP_QuitToMainMenu->GetString());

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

auto System::TypeToString(System::DeathType a_DeathType) -> std::string_view
{
    switch (a_DeathType) {
    case System::DeathType::kUnknown:
        return std::string_view{ "Unknown" };

    case System::DeathType::kDrowning:
        return std::string_view{ "Drowning" };

    case System::DeathType::kFalling:
        return std::string_view{ "Falling" };

    case System::DeathType::kPhysical:
        return std::string_view{ "Physical" };

    case System::DeathType::kMagical:
        return std::string_view{ "Magical" };

    case System::DeathType::kEnvironmentalPhysical:
        return std::string_view{ "Environmental Physical" };

    case System::DeathType::kEnvironmentalMagical:
        return std::string_view{ "Environmental Magical" };

    case System::DeathType::kScripted:
        return std::string_view{ "Scripted" };

    default:
        return std::string_view{ };
    }
}
