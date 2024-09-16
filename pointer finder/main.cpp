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
    //DWORD64 pointer = baseAddress + 0x04750A98;
    DWORD64 pointer = baseAddress + 0x0477FDB8;

    SIZE_T bytesRead;

    // Define the offsets in the pointer chain (from Cheat Engine)
    //DWORD offsets[] = { 0x0,0x0,0x50, 0x360, 0x198, 0x98, 0xD18 };
    DWORD offsets[] = { 0x80, 0x1F90, 0x18, 0xD8 };

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


DWORD64 FindPointer(HANDLE process_handle, DWORD64 base_address, int initial_offset, DWORD offsets[], int offset_amount) {
    // base address + initial offset
    DWORD64 pointer = base_address + initial_offset;
    SIZE_T bytesRead;

    //DWORD offsets[] = { 0x80, 0x1F90, 0x18, 0xD8 };
    // Iterate through the pointer chain
    for (int i = 0; i < offset_amount; i++) {
        std::cout << offsets[i] << " ";
        // Read the pointer from memory
        if (!ReadProcessMemory(process_handle, (BYTE*)pointer, &pointer, sizeof(pointer), &bytesRead) || bytesRead != sizeof(pointer)) {
            std::cerr << "Failed to read memory at offset " << std::hex << offsets[i] << std::endl;
            return 0;
        }
        pointer += offsets[i];  // Apply the offset to get to the next address in the chain
    }
    std::cout << std::endl;

    return pointer; 
}




int main() {
    const wchar_t* processName = L"DarkSoulsIII.exe";
    const wchar_t* moduleName = L"DarkSoulsIII.exe";

    // ProcessID (PID) of the game
    DWORD process_ID = GetProcessID(processName);
    if (process_ID) {

        DWORD64 baseAddress = GetModuleBaseAddress(process_ID, moduleName);
        if (baseAddress) {

            
            HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, process_ID); // Open the process with read permissions
            if (hProcess) {
                
                while (GetProcessID(processName)) {

                    //Initial offset added to base address
                    int WorldChrMan = 0x0477FDB8;

                    DWORD hp_offsets[] = { 0x80, 0x1F90, 0x18, 0xD8 };
                    int hp_offset_amount = sizeof(hp_offsets) / sizeof(hp_offsets[0]);
                    int player_health = 1;
                    DWORD64 health_pointer = FindPointer(hProcess, baseAddress, WorldChrMan, hp_offsets, hp_offset_amount);

                    DWORD animation_offsets[] = { 0x80, 0x1F90, 0x80, 0xC8 };
                    int animation_offset_amount = sizeof(animation_offsets) / sizeof(animation_offsets[0]);
                    int player_animation;
                    DWORD64 animation_pointer = FindPointer(hProcess, baseAddress, WorldChrMan, animation_offsets, animation_offset_amount);

                    int log_lock = 0;



                    //DWORD64 healthPointer = FindPlayerHealthPointer(hProcess, baseAddress);
                    if (health_pointer) {
                        while (player_health != 0) {


                            // Read the player's health value from memory

                            SIZE_T bytesRead;
                            if (ReadProcessMemory(hProcess, (BYTE*)health_pointer, &player_health, sizeof(player_health), &bytesRead) && bytesRead == sizeof(player_health)) {
                                // Successfully read the health value, print it out
                                std::cout << "Player's Health: " << player_health << std::endl;
                            }
                            else {
                                // Failed to read the health value
                                std::cerr << "Failed to read player's health!" << std::endl;
                                player_health = 0;
                            }

                            
                            if (ReadProcessMemory(hProcess, (BYTE*)animation_pointer, &player_animation, sizeof(player_animation), &bytesRead) && bytesRead == sizeof(player_animation)) {
                                // Successfully read the health value, print it out
                                std::cout << "Player's Animation: " << player_animation << std::endl;
                            }
                            else {
                                // Failed to read the health value
                                std::cerr << "Failed to read player's animation!" << std::endl;
                            }


                            if (player_animation == 68010 || player_animation == 68011 || player_animation == 68012) {
                                if (log_lock == 0) {
                                    log_lock = 1;
                                    std::cout << "Player is resting at a bonfire!" << std::endl;
                                }
                            }
                            else {
                                log_lock = 0;
                            }


                            Sleep(1000);

                        }
                        std::cout << "Player died!" << std::endl;
                        Sleep(1500);
                    }
                    else {
                        // Failed to find the health pointer
                        std::cerr << "Failed to find the health pointer!" << std::endl;
                    }

                }
                    /*
                    * 
                    999:Shadow Realm
                    4602950 : Arena : Grand Rooftop
                    4702950 : Arena : Kiln of Flame
                    5302950 : Arena : Dragon Ruins
                    5402950 : Arena : Round Plaza
                    4002950 : Firelink Shrine
                    4002959 : Ashen Grave
                    4002951 : Cemetery of Ash
                    4002952 : Iudex Gundyr
                    4002953 : Untended Graves
                    4002954 : Champion Gundyr
                    3002950 : High Wall of Lothric
                    3002955 : Tower on the Wall
                    3002952 : Vordt of the Boreal Valley
                    3002954 : Dancer of the Boreal Valley
                    3002951 : Oceiros, the Consumed King
                    3002960 : High Wall of Lothric, Teleport
                    3102954 : Foot of the High Wall
                    3102950 : Undead Settlement
                    3102952 : Cliff Underside
                    3102953 : Dilipidated Bridge
                    3102951 : Pit of Hollows
                    3302956 : Road of Sacrifices
                    3302950 : Halfway Fortress
                    3302957 : Crucifixion Woods
                    3302952 : Crystal Sage
                    3302953 : Farron Keep
                    3302954 : Keep Ruins
                    3302958 : Farron Keep Perimeter
                    3302955 : Old Wolf of Farron
                    3302951 : Abyss Watchers
                    3502953 : Cathedral of the Deep
                    3502950 : Cleansing Chapel
                    3502951 : Deacons of the Deep
                    3502952 : Rosaria's Bed Chamber
                    3802956 : Catacombs of Carthus
                    3802950 : High Lord Wolnir
                    3802951 : Abandoned Tomb
                    3802952 : Old King's Antechamber
                    3802953 : Demon Ruins
                    3802954 : Old Demon King
                    3702957 : Irithyll of the Boreal valley
                    3702954 : Central Irithyll
                    3702950 : Church of Yorshka
                    3702955 : Distant Manor
                    3702951 : Pontiff Sulyvahn
                    3702956 : Water Reserve
                    3702953 : Anor Londo
                    3702958 : Prison Tower
                    3702952 : Aldrich, Devourer of Gods
                    3902950 : Irithyll Dungeon
                    3902952 : Profaned Capital
                    3902951 : Yhorm The Giant
                    3012950 : Lothric Castle
                    3012952 : Dragon Barracks
                    3012951 : Dragonslayer Armour
                    3412951 : Grand Archives
                    3412950 : Twin Princes
                    3202950 : Archdragon Peak
                    3202953 : Dragon - Kin Mausoleum
                    3202952 : Great Belfry
                    3202951 : Nameless King
                    4102950 : Flameless Shrine
                    4102951 : Klin of the First Flame
                    4102952 : The First Flame
                    4502951 : Snowfield
                    4502952 : Rope Bridge Cave
                    4502953 : Corvian Settlement
                    4502954 : Snowy Mountain Pass
                    4502955 : Ariandel Chapel
                    4502950 : Sister Friede
                    4502957 : Depths of the Painting
                    4502956 : Champion's Gravetender
                    5002951 : The Dreg Heap
                    5002952 : Earthen Peak Ruins
                    5002953 : Within the Earthen Peak Ruins
                    5002950 : The Demon Prince
                    5102110 : The Ringed City
                    5102952 : Mausoleum Lookout
                    5102953 : Ringed Inner Wall
                    5102954 : Ringed City Streets
                    5102955 : Shared Grave
                    5102950 : Church of Filianore
                    5112951 : Filianore's rest
                    5112950 : Slave Knight Gael
                    5102951 : Darkeater Midir
                    */

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
