#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <string>

//#define DEBUG


// Function to get the process ID by name
DWORD GetProcessID(const wchar_t* process_name) {
    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        if (Process32FirstW(hProcessSnap, &pe32)) {
            do {
                if (!_wcsicmp(pe32.szExeFile, process_name)) {
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
DWORD64 GetModuleBaseAddress(DWORD processID, const wchar_t* module_name) {
    DWORD64 baseAddress = 0;
    HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
    if (hModuleSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W me32;
        me32.dwSize = sizeof(MODULEENTRY32W);
        if (Module32FirstW(hModuleSnap, &me32)) {
            do {
                if (!_wcsicmp(me32.szModule, module_name)) {
                    baseAddress = (DWORD64)me32.modBaseAddr;
                    break;
                }
            } while (Module32NextW(hModuleSnap, &me32));
        }
        CloseHandle(hModuleSnap);
    }
    return baseAddress;
}

//// Function to follow the pointer chain to the player's health address
//DWORD64 FindPlayerHealthPointer(HANDLE hProcess, DWORD64 baseAddress) {
//    // Start with base address + initial offset
//    //DWORD64 pointer = baseAddress + 0x04750A98;
//    DWORD64 pointer = baseAddress + 0x0477FDB8;
//
//    SIZE_T bytesRead;
//
//    // Define the offsets in the pointer chain (from Cheat Engine)
//    //DWORD offsets[] = { 0x0,0x0,0x50, 0x360, 0x198, 0x98, 0xD18 };
//    DWORD offsets[] = { 0x80, 0x1F90, 0x18, 0xD8 };
//
//    // Iterate through the pointer chain
//    for (int i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++) {
//        // Read the pointer from memory
//        if (!ReadProcessMemory(hProcess, (BYTE*)pointer, &pointer, sizeof(pointer), &bytesRead) || bytesRead != sizeof(pointer)) {
//            std::cerr << "Failed to read memory at offset " << std::hex << offsets[i] << std::endl;
//            return 0;
//        }
//        pointer += offsets[i];  // Apply the offset to get to the next address in the chain
//    }
//
//    return pointer;  // Final pointer to health address
//}



DWORD64 FindPointer(HANDLE process_handle, DWORD64 base_address, int initial_offset, DWORD offsets[], int offset_amount) {
    // base address + initial offset
    DWORD64 pointer = base_address + initial_offset;
    SIZE_T bytesRead;


    // Iterate through the pointer chain
    for (int i = 0; i < offset_amount; i++) {

        #ifdef DEBUG
            std::cout << offsets[i] << " ";
        #endif

        // Read the pointer from memory
        if (!ReadProcessMemory(process_handle, (BYTE*)pointer, &pointer, sizeof(pointer), &bytesRead) || bytesRead != sizeof(pointer)) {
            
                std::cerr << "Failed to read memory at offset " << std::hex << offsets[i] << std::endl;
            
            return 0;
        }
        pointer += offsets[i];  // Apply the offset to get to the next address in the chain
    }

    #ifdef DEBUG
        std::cout << std::endl;
    #endif
    

    return pointer; 
}

std::string BonfirePlace(HANDLE process_handle, DWORD64 base_address) {
    int GameMan = 0x0475AC00;
    SIZE_T bytesRead;

    DWORD bonfire_place_offsets[] = { 0xACC };
    int bonfire_place_offset_amount = sizeof(bonfire_place_offsets) / sizeof(bonfire_place_offsets[0]);
    DWORD64 bonfire_place_pointer = FindPointer(process_handle, base_address, GameMan, bonfire_place_offsets, bonfire_place_offset_amount);
    int  bonfire_ID;


    if (ReadProcessMemory(process_handle, (BYTE*)bonfire_place_pointer, &bonfire_ID, sizeof(bonfire_ID), &bytesRead) && bytesRead > 0) {
        #ifdef DEBUG
            std::cout << "Bonfire ID: " << bonfire_ID << std::endl;
        #endif
    }
    else {
        std::cerr << "Failed to read bonfire!" << std::endl << std::endl;
    }

    std::string bonfire_name = "";
    switch (bonfire_ID) {
    case 999:
        bonfire_name = "Shadow Realm";
        return bonfire_name;
    case 4602950:
        bonfire_name = "Arena : Grand Rooftop";
        return bonfire_name;
    case 4702950:
        bonfire_name = "Arena : Kiln of Flame";
        return bonfire_name;
    case 5302950:
        bonfire_name = "Arena : Dragon Ruins";
        return bonfire_name;
    case 5402950:
        bonfire_name = "Arena : Round Plaza";
        return bonfire_name;
    case 4002950:
        bonfire_name = "Firelink Shrine";
        return bonfire_name;
    case 4002959:
        bonfire_name = "Ashen Grave";
        return bonfire_name;
    case 4002951:
        bonfire_name = "Cemetery of Ash";
        return bonfire_name;
    case 4002952:
        bonfire_name = "Iudex Gundyr";
        return bonfire_name;
    case 4002953:
        bonfire_name = "Untended Graves";
        return bonfire_name;
    case 4002954:
        bonfire_name = "Champion Gundyr";
        return bonfire_name;
    case 3002950:
        bonfire_name = "High Wall of Lothric";
        return bonfire_name;
    case 3002955:
        bonfire_name = "Tower on the Wall";
        return bonfire_name;
    case 3002952:
        bonfire_name = "Vordt of the Boreal Valley";
        return bonfire_name;
    case 3002954:
        bonfire_name = "Dancer of the Boreal Valley";
        return bonfire_name;
    case 3002951:
        bonfire_name = "Oceiros, the Consumed King";
        return bonfire_name;
    case 3002960:
        bonfire_name = "High Wall of Lothric, Teleport";
        return bonfire_name;
    case 3102954:
        bonfire_name = "Foot of the High Wall";
        return bonfire_name;
    case 3102950:
        bonfire_name = "Undead Settlement";
        return bonfire_name;
    case 3102952:
        bonfire_name = "Cliff Underside";
        return bonfire_name;
    case 3102953:
        bonfire_name = "Dilipidated Bridge";
        return bonfire_name;
    case 3102951:
        bonfire_name = "Pit of Hollows";
        return bonfire_name;
    case 3302956:
        bonfire_name = "Road of Sacrifices";
        return bonfire_name;
    case 3302950:
        bonfire_name = "Halfway Fortress";
        return bonfire_name;
    case 3302957:
        bonfire_name = "Crucifixion Woods";
        return bonfire_name;
    case 3302952:
        bonfire_name = "Crystal Sage";
        return bonfire_name;
    case 3302953:
        bonfire_name = "Farron Keep";
        return bonfire_name;
    case 3302954:
        bonfire_name = "Keep Ruins";
        return bonfire_name;
    case 3302958:
        bonfire_name = "Farron Keep Perimeter";
        return bonfire_name;
    case 3302955:
        bonfire_name = "Old Wolf of Farron";
        return bonfire_name;
    case 3302951:
        bonfire_name = "Abyss Watchers";
        return bonfire_name;
    case 3502953:
        bonfire_name = "Cathedral of the Deep";
        return bonfire_name;
    case 3502950:
        bonfire_name = "Cleansing Chapel";
        return bonfire_name;
    case 3502951:
        bonfire_name = "Deacons of the Deep";
        return bonfire_name;
    case 3502952:
        bonfire_name = "Rosaria's Bed Chamber";
        return bonfire_name;
    case 3802956:
        bonfire_name = "Catacombs of Carthus";
        return bonfire_name;
    case 3802950:
        bonfire_name = "High Lord Wolnir";
        return bonfire_name;
    case 3802951:
        bonfire_name = "Abandoned Tomb";
        return bonfire_name;
    case 3802952:
        bonfire_name = "Old King's Antechamber";
        return bonfire_name;
    case 3802953:
        bonfire_name = "Demon Ruins";
        return bonfire_name;
    case 3802954:
        bonfire_name = "Old Demon King";
        return bonfire_name;
    case 3702957:
        bonfire_name = "Irithyll of the Boreal valley";
        return bonfire_name;
    case 3702954:
        bonfire_name = "Central Irithyll";
        return bonfire_name;
    case 3702950:
        bonfire_name = "Church of Yorshka";
        return bonfire_name;
    case 3702955:
        bonfire_name = "Distant Manor";
        return bonfire_name;
    case 3702951:
        bonfire_name = "Pontiff Sulyvahn";
        return bonfire_name;
    case 3702956:
        bonfire_name = "Water Reserve";
        return bonfire_name;
    case 3702953:
        bonfire_name = "Anor Londo";
        return bonfire_name;
    case 3702958:
        bonfire_name = "Prison Tower";
        return bonfire_name;
    case 3702952:
        bonfire_name = "Aldrich, Devourer of Gods";
        return bonfire_name;
    case 3902950:
        bonfire_name = "Irithyll Dungeon";
        return bonfire_name;
    case 3902952:
        bonfire_name = "Profaned Capital";
        return bonfire_name;
    case 3902951:
        bonfire_name = "Yhorm The Giant";
        return bonfire_name;
    case 3012950:
        bonfire_name = "Lothric Castle";
        return bonfire_name;
    case 3012952:
        bonfire_name = "Dragon Barracks";
        return bonfire_name;
    case 3012951:
        bonfire_name = "Dragonslayer Armour";
        return bonfire_name;
    case 3412951:
        bonfire_name = "Grand Archives";
        return bonfire_name;
    case 3412950:
        bonfire_name = "Twin Princes";
        return bonfire_name;
    case 3202950:
        bonfire_name = "Archdragon Peak";
        return bonfire_name;
    case 3202953:
        bonfire_name = "Dragon - Kin Mausoleum";
        return bonfire_name;
    case 3202952:
        bonfire_name = "Great Belfry";
        return bonfire_name;
    case 3202951:
        bonfire_name = "Nameless King";
        return bonfire_name;
    case 4102950:
        bonfire_name = "Flameless Shrine";
        return bonfire_name;
    case 4102951:
        bonfire_name = "Klin of the First Flame";
        return bonfire_name;
    case 4102952:
        bonfire_name = "The First Flame";
        return bonfire_name;
    case 4502951:
        bonfire_name = "Snowfield";
        return bonfire_name;
    case 4502952:
        bonfire_name = "Rope Bridge Cave";
        return bonfire_name;
    case 4502953:
        bonfire_name = "Corvian Settlement";
        return bonfire_name;
    case 4502954:
        bonfire_name = "Snowy Mountain Pass";
        return bonfire_name;
    case 4502955:
        bonfire_name = "Ariandel Chapel";
        return bonfire_name;
    case 4502950:
        bonfire_name = "Sister Friede";
        return bonfire_name;
    case 4502957:
        bonfire_name = "Depths of the Painting";
        return bonfire_name;
    case 4502956:
        bonfire_name = "Champion's Gravetender";
        return bonfire_name;
    case 5002951:
        bonfire_name = "The Dreg Heap";
        return bonfire_name;
    case 5002952:
        bonfire_name = "Earthen Peak Ruins";
        return bonfire_name;
    case 5002953:
        bonfire_name = "Within the Earthen Peak Ruins";
        return bonfire_name;
    case 5002950:
        bonfire_name = "The Demon Prince";
        return bonfire_name;
    case 5102110:
        bonfire_name = "The Ringed City";
        return bonfire_name;
    case 5102952:
        bonfire_name = "Mausoleum Lookout";
        return bonfire_name;
    case 5102953:
        bonfire_name = "Ringed Inner Wall";
        return bonfire_name;
    case 5102954:
        bonfire_name = "Ringed City Streets";
        return bonfire_name;
    case 5102955:
        bonfire_name = "Shared Grave";
        return bonfire_name;
    case 5102950:
        bonfire_name = "Church of Filianore";
        return bonfire_name;
    case 5112951:
        bonfire_name = "Filianore's rest";
        return bonfire_name;
    case 5112950:
        bonfire_name = "Slave Knight Gael";
        return bonfire_name;
    case 5102951:
        bonfire_name = "Darkeater Midir";
        return bonfire_name;

    default:
        std::cerr << "Failed to find bonfire name!" << std::endl;
        return "NOT FOUND";
    }

}

void convertMilliseconds(long long milliseconds) {
    long long seconds = milliseconds / 1000;        // Convert to seconds
    long long hours = seconds / 3600;               // Convert to hours
    long long minutes = (seconds % 3600) / 60;      // Convert to minutes
    long long remaining_seconds = seconds % 60;     // Remaining seconds

    std::cout << hours << ":" << minutes << ":" << remaining_seconds << std::endl;
}


int main() {
    const wchar_t* process_name = L"DarkSoulsIII.exe";
    const wchar_t* module_name = L"DarkSoulsIII.exe";

    // ProcessID (PID) of the game
    DWORD process_ID = GetProcessID(process_name);
    if (process_ID) {

        DWORD64 baseAddress = GetModuleBaseAddress(process_ID, module_name);
        if (baseAddress) {

            
            HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, process_ID); // Open the process with read permissions
            if (hProcess) {
                
                //Initial offset added to base address
                int WorldChrMan = 0x0477FDB8;
                int GameDataMan = 0x047572B8;
                SIZE_T bytesRead;

                //STEAMID

                DWORD steam_ID_offsets[] = { 0x10, 0x7D8 };
                int steam_ID_offset_amount = sizeof(steam_ID_offsets) / sizeof(steam_ID_offsets[0]);
                DWORD64 steam_ID_pointer = FindPointer(hProcess, baseAddress, GameDataMan, steam_ID_offsets, steam_ID_offset_amount);
                wchar_t  buffer[20] = { 0 };
                if (ReadProcessMemory(hProcess, (BYTE*)steam_ID_pointer, &buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {

                    std::wcout << L"Player's Steam ID: " << buffer << std::endl;
                }
                else {
                    // Failed to read the health value
                    std::cerr << "Failed to read player's name!" << std::endl << std::endl;
                }



                //CHARACTER_NAME

                DWORD character_name_offsets[] = { 0x10, 0x88 };
                int character_name_offset_amount = sizeof(character_name_offsets) / sizeof(character_name_offsets[0]);
                DWORD64 character_name_pointer = FindPointer(hProcess, baseAddress, GameDataMan, character_name_offsets, character_name_offset_amount);
                wchar_t  character_buffer[20] = { 0 };
                if (ReadProcessMemory(hProcess, (BYTE*)character_name_pointer, &character_buffer, sizeof(character_buffer), &bytesRead) && bytesRead > 0) {
                    //std::cout << "Character Name Pointer: " << std::hex << character_name_pointer << std::endl;

                    std::wcout << L"Character's name: " << character_buffer << std::endl;
                    std::cout << std::endl;
                }
                else {
                    // Failed to read the health value
                    std::cerr << "Failed to read player's name!" << std::endl << std::endl;
                }



                while (GetProcessID(process_name)) {

                    

                    //HEALTH
                    DWORD hp_offsets[] = { 0x80, 0x1F90, 0x18, 0xD8 };
                    int hp_offset_amount = sizeof(hp_offsets) / sizeof(hp_offsets[0]);
                    int player_health = 1;
                    DWORD64 health_pointer = FindPointer(hProcess, baseAddress, WorldChrMan, hp_offsets, hp_offset_amount);


                    //ANIMATION
                    DWORD animation_offsets[] = { 0x80, 0x1F90, 0x80, 0xC8 };
                    int animation_offset_amount = sizeof(animation_offsets) / sizeof(animation_offsets[0]);
                    int player_animation;
                    DWORD64 animation_pointer = FindPointer(hProcess, baseAddress, WorldChrMan, animation_offsets, animation_offset_amount);


                    //PLAYTIME
                    DWORD playtime_offsets[] = { 0xA4 };
                    int playtime_offset_amount = sizeof(playtime_offsets) / sizeof(playtime_offsets[0]);
                    long long playtime;
                    DWORD64 playtime_pointer = FindPointer(hProcess, baseAddress, GameDataMan, playtime_offsets, playtime_offset_amount);


                    //BOSSTIME
                    DWORD bosstime_offsets[] = { 0xC0 };
                    int bosstime_offset_amount = sizeof(bosstime_offsets) / sizeof(bosstime_offsets[0]);
                    unsigned in_a_bossfight;
                    DWORD64 bosstime_pointer = FindPointer(hProcess, baseAddress, GameDataMan, bosstime_offsets, bosstime_offset_amount);

                    //DEATHNUM
                    DWORD deathnum_offsets[] = { 0x98 };
                    int deathnum_offset_amount = sizeof(deathnum_offsets) / sizeof(deathnum_offsets[0]);
                    int deathnum=0;
                    DWORD64 deathnum_pointer = FindPointer(hProcess, baseAddress, GameDataMan, deathnum_offsets, deathnum_offset_amount);


                    
                    

                    int log_lock = 0;



                    //DWORD64 healthPointer = FindPlayerHealthPointer(hProcess, baseAddress);
                    if (health_pointer) {
                        while (player_health != 0 ) {

                            


                            //HEALTH
                            if (ReadProcessMemory(hProcess, (BYTE*)health_pointer, &player_health, sizeof(player_health), &bytesRead) && bytesRead == sizeof(player_health)) {
                                // Successfully read the health value, print it out
                                std::cout << "Player's Health: " << player_health << std::endl;
                            }
                            else {
                                // Failed to read the health value
                                std::cerr << "Failed to read player's health!" << std::endl;
                                player_health = 0;
                            }

                            //ANIMATION
                            if (ReadProcessMemory(hProcess, (BYTE*)animation_pointer, &player_animation, sizeof(player_animation), &bytesRead) && bytesRead == sizeof(player_animation)) {
                                // Successfully read the health value, print it out
                                std::cout << "Player's Animation: " << player_animation << std::endl;
                            }
                            else {
                                // Failed to read the health value
                                std::cerr << "Failed to read player's animation!" << std::endl;
                            }

                            //PLAYTIME
                            if (ReadProcessMemory(hProcess, (BYTE*)playtime_pointer, &playtime, sizeof(playtime), &bytesRead) && bytesRead == sizeof(playtime)) {
                                // Successfully read the health value, print it out
                                convertMilliseconds(playtime);
                            }
                            else {
                                // Failed to read the health value
                                std::cerr << "Failed to read player's playtime!" << std::endl;
                            }

                            //BOSSTIME
                            if (ReadProcessMemory(hProcess, (BYTE*)bosstime_pointer, &in_a_bossfight, sizeof(in_a_bossfight), &bytesRead) && bytesRead == sizeof(in_a_bossfight)) {
                                // Successfully read the health value, print it out
                                if(in_a_bossfight == 1) {
                                    std::cout << "Player is in a bossfight!" << std::endl;
                                }
                                else {
                                    std::cout << "Not in a bossfight!" << std::endl;
                                }
                            }
                            else {
                                // Failed to read the health value
                                std::cerr << "Failed to read player's playtime!" << std::endl;
                            }

                            if (player_animation == 68100) {
                                break;
                            }


                            if (player_animation == 68010 || player_animation == 68011 || player_animation == 68012) {
                                if (log_lock == 0) {
                                    log_lock = 1;
                                    std::string bonfire_name = BonfirePlace(hProcess, baseAddress);
                                    std::cout << "Player is resting at " <<  bonfire_name  <<" bonfire!" << std::endl;
                                    
                                }
                            }
                            else {
                                log_lock = 0;
                            }


                            std::cout << std::endl;
                            Sleep(1500);

                        }
                        Sleep(1500);
                        //DEAD
                        if (player_health == 0) {
                            if (ReadProcessMemory(hProcess, (BYTE*)deathnum_pointer, &deathnum, sizeof(deathnum), &bytesRead) && bytesRead == sizeof(deathnum)) {
                                // Successfully read the health value, print it out
                                std::cout << "Player died! [" << deathnum << "]" << std::endl;
                            }
                            else {
                                std::cout << "Player died!" << std::endl;;
                            }
                            Sleep(1500);
                        }
                    }
                    else {
                        // Failed to find the health pointer
                        std::cerr << "Failed to find the health pointer!" << std::endl;
                    }

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
