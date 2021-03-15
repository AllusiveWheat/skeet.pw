#pragma once

#include <array>
#include <string>
#include <type_traits>
#include <Windows.h>
#include <Psapi.h>
#include "ItemSchema.h"
#include <xlocmon>

class ItemSchema;

class Memory {
public:
	void initialize() noexcept;

    uintptr_t hud;
    int*(__thiscall* findHudElement)(uintptr_t, const char*);
    int(__thiscall* clearHudWeapon)(int*, int);
    std::add_pointer_t<ItemSchema* __cdecl()> itemSchema;
	int(__thiscall* equipWearable)(void* wearable, void* player);

private:
    static std::uintptr_t findPattern(const char* module , const char* pattern, size_t offset = 0) noexcept
    {
        static auto id = 0;
        ++id;

        if (MODULEINFO moduleInfo; GetModuleInformation(GetCurrentProcess(), GetModuleHandle(module), &moduleInfo, sizeof(moduleInfo))) {
            auto start = static_cast<const char*>(moduleInfo.lpBaseOfDll);
            const auto end = start + moduleInfo.SizeOfImage;

            auto first = start;
            auto second = pattern;

            while (first < end && *second) {
                if (*first == *second || *second == '?') {
                    ++first;
                    ++second;
                } else {
                    first = ++start;
                    second = pattern;
                }
            }

            if (!*second)
                return reinterpret_cast<std::uintptr_t>(const_cast<char*>(start) + offset);
        };
 
        return 0;
    }
};