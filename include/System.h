#pragma once

namespace Native
{
    static bool QuitToMainMenu()
    {
        using func_t = decltype(&QuitToMainMenu);
        REL::Relocation<func_t> function{ RELOCATION_ID(54857, 55490) };

        return function();
    }

    static void ModActorValue(RE::BSScript::Internal::VirtualMachine* a_VM, RE::VMStackID a_StackID, RE::Actor* a_Actor, RE::BSFixedString a_ActorValue, float a_Magnitude)
    {
        using func_t = decltype(&ModActorValue);
        REL::Relocation<func_t> function{ RELOCATION_ID(0, 54660) };

        return function(a_VM, a_StackID, a_Actor, a_ActorValue, a_Magnitude);
    }
}

class System
{
public:
    enum class DamageType : std::uint32_t
    {
        kUnknown = 0,
        kDrowning = 1,
        kFalling = 2,
        kPhysical = 3,
        kMagical = 4
    };

    enum class DeathType : std::uint32_t
    {
        kUnknown = 0,
        kDrowning = 1,
        kFalling = 2,
        kPhysical = 3,
        kMagical = 4,
        kEnvironmentalPhysical = 5,
        kEnvironmentalMagical = 6,
        kScripted = 7
    };

    struct Engraving
    {
        std::string source;
        std::string target;
        std::string race;
        std::string location;
        std::uint16_t level;
        float days;
        DeathType cause;
    };

    static auto GetSingleton() -> System*;

    static bool IsDead(RE::Actor* a_Actor);

    static void Delete(DeathType a_DeathType, RE::Actor* a_SourceActor, RE::Actor* a_TargetActor);

    static void QueueDeathMessage(const Engraving& a_Engraving);

    static auto TypeToString(DeathType a_DeathType) -> std::string_view;

    auto GetSaveFilesDirectory() -> std::optional<std::filesystem::path>;

    bool SkipFile(const std::string_view& a_playtime);
private:
    System() = default;
    System(const System&) = delete;
    System(System&&) = delete;

    ~System() = default;

    System& operator=(const System&) = delete;
    System& operator=(System&&) = delete;

    std::optional<std::filesystem::path> save_files;
};
