#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <iostream>

inline uintptr_t virtualaddy;

namespace mem {
    inline HANDLE driver_handle = nullptr;
    inline INT32 process_id = 0;

    inline bool find_driver() {
        if (process_id == 0) return false;
        driver_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
        return driver_handle != nullptr;
    }

    inline void read_physical(PVOID address, PVOID buffer, DWORD size) {
        SIZE_T bytesRead = 0;
        if (!driver_handle) return;
        ReadProcessMemory(driver_handle, address, buffer, size, &bytesRead);
    }

    inline void write_physical(PVOID address, PVOID buffer, DWORD size) {
        SIZE_T bytesWritten = 0;
        if (!driver_handle) return;
        WriteProcessMemory(driver_handle, address, buffer, size, &bytesWritten);
    }

    inline uintptr_t find_image() {
        uintptr_t baseAddress = 0;
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            MODULEENTRY32W me32;
            me32.dwSize = sizeof(MODULEENTRY32W);
            if (Module32FirstW(hSnapshot, &me32)) {
                do {
                    if (!_wcsicmp(me32.szModule, L"RobloxPlayerBeta.exe")) {
                        baseAddress = reinterpret_cast<uintptr_t>(me32.modBaseAddr);
                        break;
                    }
                } while (Module32NextW(hSnapshot, &me32));
            }
            CloseHandle(hSnapshot);
        }
        return baseAddress;
    }

    inline uintptr_t get_guarded_region() {
        return 0;
    }

    inline INT32 find_process(const char* process_name) {
        PROCESSENTRY32 pt;
        HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        pt.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hsnap, &pt)) {
            do {
                if (!lstrcmpi(pt.szExeFile, process_name)) {
                    CloseHandle(hsnap);
                    process_id = pt.th32ProcessID;
                    return pt.th32ProcessID;
                }
            } while (Process32Next(hsnap, &pt));
        }
        CloseHandle(hsnap);
        return 0;
    }
}

template <typename T>
inline T read(uint64_t address) {
    T buffer{ };
    mem::read_physical((PVOID)address, &buffer, sizeof(T));
    return buffer;
}

template <typename T>
T write(uint64_t address, T buffer) {
    mem::write_physical((PVOID)address, &buffer, sizeof(T));
    return buffer;
}