#include "Hooks.h"

#include "Settings.h"
#include "System.h"

#undef GetObject

namespace Hooks
{
    template <System::DamageType T>
    struct DamageHandler
    {
        static bool Call(RE::Actor* a_Target, float a_Magnitude, RE::Actor* a_Source, bool a_NoDifficultyAdjustment)
        {
            const auto result = Callback(a_Target, a_Magnitude, a_Source, a_NoDifficultyAdjustment);

            if (!System::IsDead(a_Target)) {
                return result;
            }
            
            switch (T)
            {
            case System::DamageType::kUnknown:
                System::Delete(a_Target, Settings::GetGameSetting("sAMP_Unknown"));
                break;
            case System::DamageType::kDrowning:
                System::Delete(a_Target, Settings::GetGameSetting("sAMP_Drowning"));
                break;
            case System::DamageType::kFalling:
                System::Delete(a_Target, Settings::GetGameSetting("sAMP_Falling"));
                break;
            case System::DamageType::kPhysical:
                {
                    if (a_Source && a_Target != a_Source) {
                        System::Delete(a_Target, a_Source->GetName() ? a_Source->GetName() : Settings::GetGameSetting("sAMP_DefaultSourceName"));
                    } else {
                        System::Delete(a_Target, Settings::GetGameSetting("sAMP_Environment"));
                    }
                }
                break;
            case System::DamageType::kMagical:
                {
                    if (a_Source && a_Target != a_Source) {
                        System::Delete(a_Target, a_Source->GetName() ? a_Source->GetName() : Settings::GetGameSetting("sAMP_DefaultSourceName"));
                    } else {
                        System::Delete(a_Target, Settings::GetGameSetting("sAMP_Environment"));
                    }
                }
                break;
            default:
                break;
            }

            return result;
        }
        static inline REL::Relocation<decltype(Call)> Callback;
    };

    struct ModActorValue : Xbyak::CodeGenerator
    {
        ModActorValue(std::uintptr_t a_Function, std::uintptr_t a_Address)
        {
            Xbyak::Label Function;
            Xbyak::Label Return;

            mov(rax, ptr[rip + Function]);
            jmp(ptr[rip + Return]);

            L(Function);
            dq(a_Function);

            L(Return);
            dq(a_Address + 0x7);
        }

        static void Call(RE::BSScript::Internal::VirtualMachine* a_VM, RE::VMStackID a_StackID, RE::Actor* a_Actor, RE::BSFixedString a_ActorValue, float a_Magnitude)
        {
            if (System::IsDead(a_Actor)) {

                INFO("ModActorValue::Call >> AV: {} || Magnitude: {}", a_ActorValue.c_str(), a_Magnitude);

                RE::BSTSmartPointer<RE::BSScript::Stack> stack;

                a_VM->GetStackByID(a_StackID, stack);

                std::string output;

                auto top = stack ? stack->top : nullptr;

                while (top && top->previousFrame) {
                    const auto test_owning_function = top->owningFunction ? top->owningFunction.get() : nullptr;
                    const auto test_script_name = test_owning_function ? test_owning_function->GetObjectTypeName().c_str() : Settings::GetGameSetting("sAMP_DefaultScriptName");

                    INFO("ModActorValue::Call >> Current frame: {}", test_script_name);

                    top = top->previousFrame;
                }

                // nullcheck:
                const auto owning_function = top->owningFunction ? top->owningFunction.get() : nullptr;
                const auto script_name = owning_function ? owning_function->GetObjectTypeName().c_str() : Settings::GetGameSetting("sAMP_DefaultScriptName");

                System::Delete(a_Actor, script_name);
            }

            return Native::ModActorValue(a_VM, a_StackID, a_Actor, a_ActorValue, a_Magnitude);
        }
    };

    /*
        * Although a bit unlikely to happen,
            we are blocking input for Quicksave/Quickload calls while the player is in the death scene to ensure no files are accessed while they are getting processed for deletion.
    */

    struct CanProcess
    {
        static bool Call(RE::QuickSaveLoadHandler* a_handler, RE::InputEvent* a_event)
        {
            const auto player = RE::PlayerCharacter::GetSingleton();

            if (player && player->IsDead()) {
                return false;
            }

            return Callback(a_handler, a_event);
        }
        static inline REL::Relocation<decltype(Call)> Callback;
    };

    void Install()
    {
        HOOK::CALL5<DamageHandler<System::DamageType::kDrowning>>(RELOCATE_ID(0, 37348), RELOCATE_OFFSET(0x0, 0x8EE));
        HOOK::CALL5<DamageHandler<System::DamageType::kFalling>>(RELOCATE_ID(0, 37998), RELOCATE_OFFSET(0x0, 0xC7));
        HOOK::CALL5<DamageHandler<System::DamageType::kPhysical>>(RELOCATE_ID(0, 38586), RELOCATE_OFFSET(0x0, 0x4A7));
        HOOK::CALL5<DamageHandler<System::DamageType::kMagical>>(RELOCATE_ID(0, 35086), RELOCATE_OFFSET(0x0, 0x232));

        HOOK::BRANCH5<ModActorValue, 2>(RELOCATE_ID(0, 54784), RELOCATE_OFFSET(0x0, 0x909));

        HOOK::VFUNC<RE::QuickSaveLoadHandler, 0x1, CanProcess>();

        INFO("Hooks >> Hooked <QuickSaveLoadHandler::CanProcess>");
    }
}
