#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <iomanip> 
#include <sstream> 
#include <psapi.h>
#include <string>
#include <vector>
#include <fstream>
#include "MemoryManipulation.h"
//#define DEBUG
//#define DEBUG_BOSS_VECTOR

#ifndef DataGathering
#define DataGathering

/**
 * @brief This function stores all the preset initial offsets, depending on what you need to use
 *
 * @param 4 options: WorldChrMan , GameDataMan , GameMan , SprjEventFlagMan
 *
 * @returns The initial offset, that needs to be added to base address
 */
int GetInitialOffset(std::string name) {
	if (name == "WorldChrMan") {
		return 0x0477FDB8;
	}
	if (name == "GameDataMan") {
		return 0x047572B8;
	}
	if (name == "GameMan") {
		return 0x0475AC00;
	}
	if (name == "SprjEventFlagMan") {
		return 0x04752F68;
	}
}


/**
 * @brief It gives back a specific pointer, according to what number is given
 *
 * @param process_handle  The process handle
 * @param base_address  The base address
 * @param number  0 Steam ID ; 1 Character name ; 2 Current health ; 3 Current animation ;
 * 4 Playtime ; 5 Number of deaths ; 6 Currently fighting a boss? ; 7 Last Bonfire ;
 * 8 Current souls ; 9 Total souls ; 10 Level
 *
 * @returns A pointer to the desired place
 */
DWORD64 FindSpecificPointer(HANDLE process_handle, DWORD64 base_address, int number) {
	std::vector<DWORD> offsets;
	//offsets.resize(21);
	int offset_amount;
	DWORD64 pointer;
	int initial_offset;

	switch (number) {
	case 0: //STEAM ID
		offsets.push_back(0x10);
		offsets.push_back(0x7D8);
		offset_amount = offsets.capacity();
		initial_offset = GetInitialOffset("GameDataMan");
		break;

	case 1: //CHARACTER NAME
		offsets.push_back(0x10);
		offsets.push_back(0x88);
		offset_amount = offsets.capacity();
		initial_offset = GetInitialOffset("GameDataMan");
		break;

	case 2: //CURRENT HEALTH
		offsets.push_back(0x80);
		offsets.push_back(0x1F90);
		offsets.push_back(0x18);
		offsets.push_back(0xD8);
		offset_amount = offsets.capacity();
		initial_offset = GetInitialOffset("WorldChrMan");
		break;

	case 3: //CURRENT ANIMATION
		offsets.push_back(0x80);
		offsets.push_back(0x1F90);
		offsets.push_back(0x80);
		offsets.push_back(0xC8);
		offset_amount = offsets.capacity();
		initial_offset = GetInitialOffset("WorldChrMan");
		break;

	case 4: //PLAYTIME
		offsets.push_back(0xA4);
		offset_amount = offsets.capacity();
		initial_offset = GetInitialOffset("GameDataMan");
		break;

	case 5: //NUMBER OF DEATHS
		offsets.push_back(0x98);
		offset_amount = offsets.capacity();
		initial_offset = GetInitialOffset("GameDataMan");
		break;

	case 6: //"CURRENTLY FIGHTING A BOSS?"
		offsets.push_back(0xC0);
		offset_amount = offsets.capacity();
		initial_offset = GetInitialOffset("GameDataMan");
		break;

	case 7: //BONFIRE PLACE
		offsets.push_back(0xACC);
		offset_amount = offsets.capacity();
		initial_offset = GetInitialOffset("GameMan");
		break;

	case 8: //CURRENT SOULS
		offsets.push_back(0x10);
		offsets.push_back(0x74);
		offset_amount = offsets.capacity();
		initial_offset = GetInitialOffset("GameDataMan");
		break;

	case 9: //TOTAL SOULS
		offsets.push_back(0x10);
		offsets.push_back(0x78);
		offset_amount = offsets.capacity();
		initial_offset = GetInitialOffset("GameDataMan");
		break;

	case 10: //LEVEL
		offsets.push_back(0x10);
		offsets.push_back(0x70);
		offset_amount = offsets.capacity();
		initial_offset = GetInitialOffset("GameDataMan");
		break;


	default:
		std::cerr << "No preset pointer with this number!" << std::endl;
		return NULL;
	}
	pointer = FindPointer(process_handle, base_address, initial_offset, offsets, offset_amount);
	return pointer;

}


/**
 * @brief Finds the character's name and the SteamID of the player
 *
 * @returns Character's_name[SteamID].txt
 */
std::string GetPlayerID(HANDLE process_handle, DWORD64 base_address) {

	SIZE_T bytesRead;
	std::string PlayerID = "";
	std::string CharacterName = "";

	//CHARACTER_NAME 1
	DWORD64 character_name_pointer = FindSpecificPointer(process_handle, base_address, 1);
	wchar_t  character_buffer[20] = { 0 };
	if (ReadProcessMemory(process_handle, (BYTE*)character_name_pointer, &character_buffer, sizeof(character_buffer), &bytesRead) && bytesRead > 0) {
#ifdef DEBUG
		std::cout << "Character Name Pointer: " << std::hex << character_name_pointer << std::endl;
		std::wcout << L"Character's name: " << character_buffer << std::endl;
#endif 

		size_t size_needed = wcstombs(nullptr, character_buffer, 0) + 1;
		char* narrow_str = new char[size_needed];
		//Conversion wide -> char*
		wcstombs(narrow_str, character_buffer, size_needed);
		std::string CharacterName(narrow_str);
		PlayerID = PlayerID + CharacterName;
		delete[] narrow_str;
	}
	else {
#ifdef DEBUG
		std::cerr << "Failed to read player's name!" << std::endl << std::endl;
#endif
	}

	//STEAMID 0
	DWORD64 steam_ID_pointer = FindSpecificPointer(process_handle, base_address, 0);
	wchar_t  buffer[20] = { 0 };
	if (ReadProcessMemory(process_handle, (BYTE*)steam_ID_pointer, &buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {

#ifdef DEBUG
		std::cout << "Steam ID Pointer: " << std::hex << steam_ID_pointer << std::endl;
		std::wcout << L"Player's Steam ID: " << buffer << std::endl;
		std::cout << std::endl;
#endif

		size_t size_needed = wcstombs(nullptr, buffer, 0) + 1;
		char* narrow_str = new char[size_needed];
		//Conversion wide -> char*
		wcstombs(narrow_str, buffer, size_needed);
		std::string SteamID(narrow_str);
		PlayerID = PlayerID + "[" + SteamID + "].txt";
		delete[] narrow_str;
	}
	else {
#ifdef DEBUG
		std::cerr << "Failed to read player's Steam ID!" << std::endl << std::endl;
#endif
	}

#ifdef DEBUG
	std::cout << PlayerID << std::endl << std::endl;
#endif

	
	return PlayerID;
}


/**
 * @brief Scan's the memory for the last bonfire
 *
 * @returns Name of last bonfire rested at
 */
std::string BonfirePlace(HANDLE process_handle, DWORD64 base_address) {
	SIZE_T bytesRead;


	//DWORD bonfire_place_offsets[] = { 0xACC };
	//int bonfire_place_offset_amount = sizeof(bonfire_place_offsets) / sizeof(bonfire_place_offsets[0]);
	//DWORD64 bonfire_place_pointer = FindPointer(process_handle, base_address, GetInitialOffset("GameMan"), bonfire_place_offsets, bonfire_place_offset_amount);
	DWORD64 bonfire_place_pointer = FindSpecificPointer(process_handle, base_address, 7);
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



/**
 * @brief Scans all the boss flags
 *
 * @returns a vector containing info about bosses (defeated or not)
 */
std::vector<int> GetBossVector(HANDLE process_handle, DWORD64 base_address) {
	SIZE_T bytesRead;
	std::vector<int> boss_vector;
	int initial_offset = GetInitialOffset("SprjEventFlagMan");

#ifdef DEBUG_BOSS_VECTOR
	std::cout << "[";
#endif

	for (int i = 0; i < 24; i++) {
		std::vector<DWORD>offsets;
		int offset_amount;
		int bit_to_push;

		switch (i) {
		case 0: //Iudex Gundyr
			offsets.push_back(0x0);
			offsets.push_back(0x5A67);
			bit_to_push = 7;
			break;

		case 1: //Vordt of the Boreal Valley
			offsets.push_back(0x0);
			offsets.push_back(0xF67);
			bit_to_push = 7;
			break;

		case 2: //Curse-Rotted Greatwood
			offsets.push_back(0x0);
			offsets.push_back(0x1967);
			bit_to_push = 7;
			break;

		case 3: //Crystal Sage
			offsets.push_back(0x0);
			offsets.push_back(0x2D69);
			bit_to_push = 5;
			break;

		case 4: //Deacons of the Deep
			offsets.push_back(0x0);
			offsets.push_back(0x3C67);
			bit_to_push = 7;
			break;

		case 5: //Abyss Watchers
			offsets.push_back(0x0);
			offsets.push_back(0x2D67);
			bit_to_push = 7;
			break;

		case 6: //High Lord Wolnir
			offsets.push_back(0x0);
			offsets.push_back(0x5067);
			bit_to_push = 7;
			break;

		case 7: //Old Demon King
			offsets.push_back(0x0);
			offsets.push_back(0x5064);
			bit_to_push = 1;
			break;

		case 8: //Yhorm the Giant
			offsets.push_back(0x0);
			offsets.push_back(0x5567);
			bit_to_push = 7;
			break;

		case 9: //Pontiff Sulyvahn
			offsets.push_back(0x0);
			offsets.push_back(0x4B69);
			bit_to_push = 5;
			break;

		case 10: //Aldrich, Devourer of Gods
			offsets.push_back(0x0);
			offsets.push_back(0x4B67);
			bit_to_push = 7;
			break;

		case 11: //Dancer of the Boreal Valley
			offsets.push_back(0x0);
			offsets.push_back(0xF6C);
			bit_to_push = 5;
			break;

		case 12: //Oceiros, the Consumed King
			offsets.push_back(0x0);
			offsets.push_back(0xF64);
			bit_to_push = 1;
			break;

		case 13: //Champion Gundyr
			offsets.push_back(0x0);
			offsets.push_back(0x5A64);
			bit_to_push = 1;
			break;

		case 14: //Nameless King
			offsets.push_back(0x0);
			offsets.push_back(0x2369);
			bit_to_push = 5;
			break;

		case 15: //Dragonslayer Armour
			offsets.push_back(0x0);
			offsets.push_back(0x1467);
			bit_to_push = 7;
			break;

		case 16: //Twin Princes
			offsets.push_back(0x0);
			offsets.push_back(0x3764);
			bit_to_push = 1;
			break;

		case 17: //Soul of Cinder
			offsets.push_back(0x0);
			offsets.push_back(0x5F67);
			bit_to_push = 7;
			break;

		case 18: //Champion's Gravetender
			offsets.push_back(0x0);
			offsets.push_back(0x6468);
			bit_to_push = 3;
			break;

		case 19: //Father Ariandel and Sister Friede
			offsets.push_back(0x0);
			offsets.push_back(0x6467);
			bit_to_push = 7;
			break;

		case 20: //Halflight, Spear of the Church
			offsets.push_back(0x0);
			offsets.push_back(0x7867);
			bit_to_push = 7;
			break;

		case 21: //Darkeater Midir
			offsets.push_back(0x0);
			offsets.push_back(0x7869);
			bit_to_push = 5;
			break;

		case 22: //Slave Knight Gael
			offsets.push_back(0x0);
			offsets.push_back(0x7D67);
			bit_to_push = 7;
			break;

		case 23: //Demon Prince
			offsets.push_back(0x0);
			offsets.push_back(0x7367);
			bit_to_push = 7;
			break;


		}
		offset_amount = offsets.capacity();
		DWORD64 pointer = FindPointer(process_handle, base_address, initial_offset, offsets, offset_amount);
		BYTE flagByte = 0;
		//if (ReadProcessMemory(process_handle, (BYTE*)pointer, &flagByte, sizeof(flagByte), &bytesRead) && bytesRead > 0){
		if (ReadProcessMemory(process_handle, (BYTE*)pointer, &flagByte, sizeof(flagByte), &bytesRead) && bytesRead == sizeof(flagByte)) {
			bool isFlagSet = (flagByte & (1 << bit_to_push)) != 0;
			if (isFlagSet) {
				boss_vector.push_back(1);

#ifdef DEBUG_BOSS_VECTOR
				std::cout << true << " , ";
#endif				
		}
			else {
				boss_vector.push_back(0);

#ifdef DEBUG_BOSS_VECTOR
				std::cout << false << " , ";
#endif 
	}

}
		else {
			std::cerr << "Failed to read the flag value!" << std::endl;
			boss_vector.push_back(2);
		}

		}
#ifdef DEBUG_BOSS_VECTOR
	std::cout << "]" << std::endl;
#endif 

	return boss_vector;
}



/**
 * @brief Compares 2 boss vectors, looks for difference
 *
 * @returns If there is a difference, it means a boss was defeated
 */
std::string CompareBossVectors(std::vector<int> boss_vector_start, std::vector<int> boss_vector_end) {
	int boss_num = -1;
	for (int i = 0; i < boss_vector_start.capacity(); i++) {
		if (!(boss_vector_start.at(i) == boss_vector_end.at(i))) {
			boss_num = i;
			break;
		}

	}
	std::string boss_name = "0";
	switch (boss_num) {
	case -1:
		std::cout << "Error finding boss" << std::endl;
		break;
	case 0:
		boss_name = "Iudex Gundyr";
		break;
	case 1:
		boss_name = "Vordt of the Boreal Valley";
		break;
	case 2:
		boss_name = "Curse-Rotted Greatwood";
		break;
	case 3:
		boss_name = "Crystal Sage";
		break;
	case 4:
		boss_name = "Deacons of the Deep";
		break;
	case 5:
		boss_name = "Abyss Watchers";
		break;
	case 6:
		boss_name = "High Lord Wolnir";
		break;
	case 7:
		boss_name = "Old Demon King";
		break;
	case 8:
		boss_name = "Yhorm the Giant";
		break;
	case 9:
		boss_name = "Pontiff Sulyvahn";
		break;
	case 10:
		boss_name = "Aldrich, Devourer of Gods";
		break;
	case 11:
		boss_name = "Dancer of the Boreal Valley";
		break;
	case 12:
		boss_name = "Oceiros, the Consumed King";
		break;
	case 13:
		boss_name = "Champion Gundyr";
		break;
	case 14:
		boss_name = "Nameless King";
		break;
	case 15:
		boss_name = "Dragonslayer Armour";
		break;
	case 16:
		boss_name = "Twin Princes";
		break;
	case 17:
		boss_name = "Soul of Cinder";
		break;
	case 18:
		boss_name = "Champion's Gravetender";
		break;
	case 19:
		boss_name = "Father Ariandel and Sister Friede";
		break;
	case 20:
		boss_name = "Halflight, Spear of the Church";
		break;
	case 21:
		boss_name = "Darkeater Midir";
		break;
	case 22:
		boss_name = "Slave Knight Gael";
		break;
	case 23:
		boss_name = "Demon Prince";
		break;
	}

	//if (boss_num != -1) {
	//	std::cout << "Player defeated " << boss_name << std::endl;
	//}

	return boss_name;
}



/**
 * @brief Converts miliseconds that were from the memroy address
 *
 * @returns a full string with format HH:MM:SS
 */
std::string convertMilliseconds(long long milliseconds) {
#ifdef DEBUG
	std::cout << milliseconds << std::endl; //<< std::dec;
#endif

	long long seconds = milliseconds / 1000;        // Convert to seconds
	int hours = static_cast<int>(seconds / 3600);               // Convert to hours
	int minutes = static_cast<int> ((seconds % 3600) / 60);      // Convert to minutes
	int remaining_seconds = static_cast<int> (seconds % 60);     // Remaining seconds

	std::string divider = ":";
	//std::string playtime = std::to_string(hours) + divider + std::to_string(minutes) + divider + std::to_string(remaining_seconds);
	std::stringstream temp;

	// Set width to 2, fill with '0', and convert to string
	temp << std::setw(2) << std::setfill('0') << std::to_string(hours) << divider
		<< std::setw(2) << std::setfill('0') << std::to_string(minutes) << divider
		<< std::setw(2) << std::setfill('0') << std::to_string(remaining_seconds);

	std::string playtime = temp.str();
#ifdef DEBUG
	std::cout << playtime << std::endl;
#endif

	return playtime;
}

/**
 * @brief Writes to a log file
 */
void WriteToLog(std::string filename, std::string message) {
	std::ofstream My_file;
	My_file.open(filename, std::ios::app);
	My_file << message;
	My_file.close();
	/*
	* Explanation for the logging:
	*
	* #
	* death number
	* when it happened
	*
	* ##
	* resting at which bonfire
	* current level
	* current souls
	* total souls
	* when it happened
	*
	* ###
	* recovered souls
	* when it happened
	*
	* ####
	* ENTERED / DIED / DEFEATED (bossfights)
	* boss name / NIL
	* when it happened
	* Boss name
	*
	* #####
	* Sat down to play at what time
	*/
}

#endif // !DataGathering

