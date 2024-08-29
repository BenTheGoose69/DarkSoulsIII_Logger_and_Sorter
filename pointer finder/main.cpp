#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

// Function to get the process ID by name
DWORD GetProcessID(const wchar_t* processName) {
    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        if (Process32FirstW(hProcessSnap, &pe32)) {
            do {
                if (!_wcsicmp(pe32.szExeFile, processName)) {
                    processID = pe32.th32ProcessID;
                    break;
                }
            } while (Process32NextW(hProcessSnap, &pe32));
        }
        CloseHandle(hProcessSnap);
    }
    return processID;
}

// Function to get the base address of the module (DarkSoulsIII.exe)
DWORD64 GetModuleBaseAddress(DWORD processID, const wchar_t* moduleName) {
    DWORD64 baseAddress = 0;
    HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
    if (hModuleSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W me32;
        me32.dwSize = sizeof(MODULEENTRY32W);
        if (Module32FirstW(hModuleSnap, &me32)) {
            do {
                if (!_wcsicmp(me32.szModule, moduleName)) {
                    baseAddress = (DWORD64)me32.modBaseAddr;
                    break;
                }
            } while (Module32NextW(hModuleSnap, &me32));
        }
        CloseHandle(hModuleSnap);
    }
    return baseAddress;
}

// Function to follow the pointer chain to the player's health address
DWORD64 FindPlayerHealthPointer(HANDLE hProcess, DWORD64 baseAddress) {
    // Start with base address + initial offset
    DWORD64 pointer = baseAddress + 0x04750A98;
    SIZE_T bytesRead;

    // Define the offsets in the pointer chain (from Cheat Engine)
    DWORD offsets[] = { 0x0,0x0,0x50, 0x360, 0x198, 0x98, 0xD18 };

    // Iterate through the pointer chain
    for (int i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++) {
        // Read the pointer from memory
        if (!ReadProcessMemory(hProcess, (BYTE*)pointer, &pointer, sizeof(pointer), &bytesRead) || bytesRead != sizeof(pointer)) {
            std::cerr << "Failed to read memory at offset " << std::hex << offsets[i] << std::endl;
            return 0;
        }
        pointer += offsets[i];  // Apply the offset to get to the next address in the chain
    }

    return pointer;  // Final pointer to health address
}

int main() {
    // Define the process name and module name
    const wchar_t* processName = L"DarkSoulsIII.exe";
    const wchar_t* moduleName = L"DarkSoulsIII.exe";

    // Get the process ID of the game
    DWORD processID = GetProcessID(processName);
    if (processID) {
        // Get the base address of the game module
        DWORD64 baseAddress = GetModuleBaseAddress(processID, moduleName);
        if (baseAddress) {
            // Open the process with read permissions
            HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processID);
            if (hProcess) {
                // Find the health pointer using the base address and offsets
                DWORD64 healthPointer = FindPlayerHealthPointer(hProcess, baseAddress);
                if (healthPointer) {
                    // Read the player's health value from memory
                    int playerHealth;
                    SIZE_T bytesRead;
                    if (ReadProcessMemory(hProcess, (BYTE*)healthPointer, &playerHealth, sizeof(playerHealth), &bytesRead) && bytesRead == sizeof(playerHealth)) {
                        // Successfully read the health value, print it out
                        std::cout << "Player's Health: " << playerHealth << std::endl;
                    }
                    else {
                        // Failed to read the health value
                        std::cerr << "Failed to read player's health!" << std::endl;
                    }
                }
                else {
                    // Failed to find the health pointer
                    std::cerr << "Failed to find the health pointer!" << std::endl;
                }
                // Close the handle to the process
                CloseHandle(hProcess);
            }
            else {
                // Failed to open the process
                std::cerr << "Failed to open process!" << std::endl;
            }
        }
        else {
            // Failed to get the base address
            std::cerr << "Failed to get base address!" << std::endl;
        }
    }
    else {
        // Failed to get the process ID
        std::cerr << "Failed to find process ID!" << std::endl;
    }

    return 0;
}
