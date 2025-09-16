#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h> 

#include <REX/W32.h>
#include "REX/W32/SHELL32.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include <toml.hpp>
#include <xbyak/xbyak.h>

using namespace std::literals;

#define ERROR(message, ...) SKSE::log::error(message, ##__VA_ARGS__)
#define INFO(message, ...) SKSE::log::info(message, ##__VA_ARGS__)
#define TRACE(message, ...) SKSE::log::trace(message, ##__VA_ARGS__)
#define WARN(message, ...) SKSE::log::warn(message, ##__VA_ARGS__)

#define RELOCATE_ID(a_15, a_16) REL::RelocationID(a_15, a_16)
#define RELOCATE_OFFSET(a_15, a_16) REL::Relocate<std::uint32_t>(a_15, a_16)

namespace HOOK
{
    template <class T>
    void CALL5(REL::RelocationID a_ID, std::uint32_t a_Offset)
    {
        const REL::Relocation target{ a_ID, a_Offset };

        auto& trampoline = SKSE::GetTrampoline();
        T::Callback = trampoline.write_call<5>(target.address(), T::Call);
    }

    template <class T>
    void CALL6(REL::RelocationID a_ID, std::uint32_t a_Offset)
    {
        const REL::Relocation target{ a_ID, a_Offset };

        auto& trampoline = SKSE::GetTrampoline();
        T::Callback = *reinterpret_cast<std::uintptr_t*>(trampoline.write_call<6>(target.address(), T::Call));
    }

    template <class T, std::uint32_t index, class U>
    void VFUNC()
    {
        REL::Relocation VTABLE{ T::VTABLE[0] };
        U::Callback = VTABLE.write_vfunc(index, U::Call);
    }

    template <class T, std::uint32_t table, std::uint32_t index, class U>
    void VFUNC()
    {
        REL::Relocation VTABLE{ T::VTABLE[table] };
        U::Callback = VTABLE.write_vfunc(index, U::Call);
    }

    template <std::integral T, std::size_t N>
    void WRITE_BYTES(std::uintptr_t a_Destination, const std::array<T, N>& a_Data)
    {
        REL::safe_write(a_Destination, a_Data.data(), a_Data.size() * sizeof(T));
    }

    template <class T, std::size_t NOPs = 0>
    void BRANCH5(REL::RelocationID a_ID, std::uint32_t a_Offset)
    {
        const REL::Relocation target{ a_ID, a_Offset };

        T hook(reinterpret_cast<std::uintptr_t>(T::Call), target.address());
        hook.ready();

        auto& trampoline = SKSE::GetTrampoline();

        trampoline.write_branch<5>(target.address(), trampoline.allocate(hook));

        if constexpr (NOPs) {
            std::array<std::uint8_t, NOPs> buffer{};
            buffer.fill(0x90);

            WRITE_BYTES(target.address() + 0x5, buffer);
        }
    }
}
