#include "Hooks.h"

#include "Data.h"

namespace Hooks
{
    struct DeathHandler 
    {
        static bool thunk(RE::Character* a_target, RE::Character* a_source)
        {
            if (a_target == RE::PlayerCharacter::GetSingleton()) {
                const auto save_manager = RE::BGSSaveLoadManager::GetSingleton();

                if (save_manager) {
                    save_manager->BuildSaveGameList();

                    INFO("Hooks::DeathHandler ~ Processing death for character: <{:X}>", save_manager->currentCharacterID);
                    
                    const auto data = Data::GetSingleton();

                    const auto save_files = data->GetSaveFileDirectory();

                    if (!save_files) { stl::report_and_fail("Failed to obtain save files path!"); }

                    const auto worldspace = a_target->GetWorldspace();

                    // DLC2ApocryphaWorld
                    if (worldspace && worldspace->GetFormID() == 0x0401C0B2) {
                        INFO("Hooks::DeathHandler ~ Character: <{:X}> died in DLC2ApocryphaWorld! Ceasing the plugin's functions...", save_manager->currentCharacterID);

                        return func(a_target, a_source);
                    }

                    const auto source_name = a_source ? a_source->GetName() : "???";
                    const auto target_name = a_target ? a_target->GetName() : "Bernard";
                    const auto race = a_target->GetRace()->GetName();
                    const auto level = a_target->GetLevel();
                    const auto location = a_target->GetCurrentLocation() ? a_target->GetCurrentLocation()->GetName() : worldspace ? worldspace->GetName() : "Tamriel";

                    const auto calendar = RE::Calendar::GetSingleton();

                    const auto days_passed = calendar ? std::floorf(calendar->GetDaysPassed()) : 0.f;

                    data->QueueMessage(source_name, target_name, location, race, level, days_passed);
                    data->WriteOutput(source_name, target_name, location, race, level, days_passed);

                    try {
                        if (std::filesystem::exists(*save_files)) {
                            for (const auto& file : save_manager->saveGameList) {
                                file->PopulateFileEntryData();

                                if (save_manager->currentCharacterID == 0x0 || (file->characterID == 0x0 && file->characterName != target_name)) { continue; };

                                if (file->characterID == save_manager->currentCharacterID) {

                                    // Skip the deletion of save files with a play time lower than the current threshold

                                    if (data->SkipFile(file->playTime)) { 
                                        INFO("Hooks::DeathHandler ~ Skipping file: <{}> with a play time of: <{}>", file->fileName.c_str(), file->playTime.c_str());
                                        continue; 
                                    };

                                    for (const auto& entry : std::filesystem::directory_iterator(*save_files)) {
                                        if (entry.path().stem() == file->fileName.data()) {
                                            INFO("Hooks::DeathHandler ~ Deleting file: <{}> with a play time of: <{}>", file->fileName.c_str(), file->playTime.c_str());

                                            std::filesystem::remove(entry);
                                        }
                                    }
                                }
                            }
                        }
                    } catch (const std::filesystem::filesystem_error& error) {
                        stl::report_and_error(error.what());
                    }
                }
            }

            return func(a_target, a_source);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    /*
        * Although a bit unlikely to happen,
            we are blocking input for Quicksave/Quickload calls while the player is in the death scene to ensure no files are accessed while they are getting processed for deletion.
    */

    struct CanProcess
    {
        static bool thunk(RE::QuickSaveLoadHandler* a_handler, RE::InputEvent* a_event)
        {
            const auto player = RE::PlayerCharacter::GetSingleton();

            if (player && player->IsDead()) {
                return false;
            }

            return func(a_handler, a_event);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    void Install()
    {
        REL::Relocation death_handler{ RELOCATION_ID(36872, 37896), REL::Relocate(0x588, 0x5F8) };
        stl::write_thunk_call<DeathHandler>(death_handler.address());

        INFO("Hooks ~ Hooked <DeathHandler>");

        stl::write_vfunc<RE::QuickSaveLoadHandler, 0x1, CanProcess>();

        INFO("Hooks ~ Hooked <QuickSaveLoadHandler::CanProcess>");
    }
}
