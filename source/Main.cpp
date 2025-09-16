#include "Settings.h"
#include "System.h"
#include "Hooks.h"

/*
 * Migrate to TOML over INI.
 * Include a method of increasing the amount of maximum lives, perhaps through a internal counter that can be accessed through scripts? Maybe a TESGlobal?
 * Refactor some parts of the code to use my own updated macro definitions.
 * Change how death detection is done completely, allowing different types of death such as falling damage, drowning, traps or even scripted deaths to be detected by this plugin.
 * I don't like the existence of Engravings.txt; Perhaps implement a way to generate a unique hash or similar data structure that can be parsed into the character's information in my own website?
 * Have a list of pre-defined death phrases for supported ways to die.
 * Move translations to optional .esp files instead of TOML.
 * 
 * Falling  Damage: 69F600+C7
 * Drowning Damage: 667D40+8EE
 * 
 * Magical Damage: 5D8A70+232
 * Physical Damage: 6B7BC0+4A7
 * 
 * Trap     Damage: A2F3B0+FD (TESObjectREFR::DoTrap1() 6 bytes vfunc call). Need to RE the second argument structure. NOT GOOD ENOUGH.
 */

void InitializeLogger()
{
    auto path = SKSE::log::log_directory();

    if (!path) { return; }

    const auto plugin = SKSE::PluginDeclaration::GetSingleton();
    *path /= std::format("{}.log", plugin->GetName());

    std::vector<spdlog::sink_ptr> sinks{
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true),
        std::make_shared<spdlog::sinks::msvc_sink_mt>()
    };

    auto logger = std::make_shared<spdlog::logger>("global", sinks.begin(), sinks.end());

    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::move(logger));
    spdlog::set_pattern("[%^%L%$] %v");
}

void HandleMessage(SKSE::MessagingInterface::Message* a_message)
{
    switch (a_message->type) {
    case SKSE::MessagingInterface::kInputLoaded:
        {
            Settings::Load();
            Hooks::Install();
        }
        break;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    InitializeLogger();

    //while (!IsDebuggerPresent()) Sleep(100);

    SKSE::Init(a_skse);

    SKSE::AllocTrampoline(14 * 8);

    const auto messaging_interface = SKSE::GetMessagingInterface();

    if (!messaging_interface) { SKSE::stl::report_and_fail("SKSEPluginLoad ~ Failed to communicate with the messaging interface!"); }

    messaging_interface->RegisterListener(HandleMessage);

    return true;
}
