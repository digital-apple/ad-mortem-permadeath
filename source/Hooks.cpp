#include "Hooks.h"

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
                System::Delete(System::DeathType::kUnknown, a_Source, a_Target);
                break;
            case System::DamageType::kDrowning:
                System::Delete(System::DeathType::kDrowning, a_Source, a_Target);
                break;
            case System::DamageType::kFalling:
                System::Delete(System::DeathType::kFalling, a_Source, a_Target);
                break;
            case System::DamageType::kPhysical:
                {
                    System::Delete(a_Source && a_Source != a_Target ?
                        System::DeathType::kPhysical :
                        System::DeathType::kEnvironmentalPhysical,
                    a_Source, a_Target);
                }
                break;
            case System::DamageType::kMagical:
                {
                    System::Delete(a_Source && a_Source != a_Target ?
                        System::DeathType::kMagical :
                        System::DeathType::kEnvironmentalMagical,
                    a_Source, a_Target);
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
                System::Delete(System::DeathType::kScripted, nullptr, a_Actor);

                RE::BSTSmartPointer<RE::BSScript::Stack> stack;

                a_VM->GetStackByID(a_StackID, stack);

                std::string output;

                auto top = stack ? stack->top : nullptr;

                while (top) {
                    const auto owning_function = top->owningFunction ? top->owningFunction.get() : nullptr;
                    const auto script_name = owning_function ? owning_function->GetObjectTypeName().c_str() : "NONE";
                    const auto function_name = owning_function ? owning_function->GetName().c_str() : "NONE";

                    std::string frame = std::format("{}::{}()", script_name, function_name);

                    if (output.empty()) {
                        output = frame;
                    } else {
                        output = frame + "->" + output;
                    }

                    top = top->previousFrame;
                }
                
                if (!output.empty()) { INFO("ModActorValue >> {}", output); }
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

        INFO("Hooks ~ Hooked <QuickSaveLoadHandler::CanProcess>");
    }
}
