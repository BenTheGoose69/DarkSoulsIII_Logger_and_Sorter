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
#include "DataGathering.h"
//#define DEBUG

int main() {

	std::cout << R"( 

   _        _______  _______      _______  _______           _        _______ 
  ( \      (  ___  )(  ____ \    (  ____ \(  ___  )|\     /|( \      (  ____ \
  | (      | (   ) || (    \/    | (    \/| (   ) || )   ( || (      | (    \/
  | |      | |   | || |          | (_____ | |   | || |   | || |      | (_____ 
  | |      | |   | || | ____     (_____  )| |   | || |   | || |      (_____  )
  | |      | |   | || | \_  )          ) || |   | || |   | || |            ) |
  | (____/\| (___) || (___) |    /\____) || (___) || (___) || (____/\/\____) |
  (_______/(_______)(_______)    \_______)(_______)(_______)(_______/\_______)

    )" << std::endl;

	const wchar_t* process_name = L"DarkSoulsIII.exe";
	const wchar_t* module_name = L"DarkSoulsIII.exe";
	DWORD process_ID;

	/*
	Used to make pretty loading
	0-3 -> loading
	-1  --> game started
	*/
	int game_started = 0;

	//
	//	GAME START CHECK
	//

	while (game_started != -1) {
		// ProcessID (PID) of the game
		process_ID = GetProcessID(process_name);


		if (process_ID) {

			game_started = -1;
			std::cout << "\rWaiting for the game to be launched... Game started!" << std::endl;
			Sleep(3000);

		}
		else {
			//waiting_for_pointers to launch the game
			switch (game_started)
			{
			case 0:
				std::cout << "\rWaiting for the game to be launched   ";
				std::cout << "\rWaiting for the game to be launched";
				game_started++;
				break;
			case 1:
				std::cout << ".";
				game_started++;
				break;
			case 2:
				std::cout << ".";
				game_started++;
				break;
			case 3:
				std::cout << ".";
				game_started = 0;
				break;
			default:
				break;
			}

			Sleep(800);


			// Failed to get the process ID
			//std::cout << "Failed to find process ID!" << std::endl;
		}
	}

	//
	// BASE ADDRESS CHECK
	//

	DWORD64 baseAddress = GetModuleBaseAddress(process_ID, module_name);
	if (!baseAddress) {
		std::cout << "Failed to get base address!" << std::endl;
		return 0;
	}

	//
	// OPEN PROCESS CHECK
	//

	// Open the process with read permissions
	HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, process_ID);
	if (!hProcess) {
		std::cout << "Failed to open process!" << std::endl;
		return 0;
	}

	//
	// WHILE THE GAME IS RUNING
	//

	//Used for reading bytes
	SIZE_T bytesRead;
	//Used to make pretty loading for pointers
	int waiting_for_pointers = 0;
	//Death counter already printed to console
	int printed_death = 0;
	//Helps loging the first launch / playtime calculations
	int launchTime = 0;

	while (GetProcessID(process_name)) {
		std::string Player_ID = GetPlayerID(hProcess, baseAddress);
		int log_lock = 0;

		//HEALTH 2
		DWORD64 health_pointer = FindSpecificPointer(hProcess, baseAddress, 2);
		int player_health = 1;

		//ANIMATION 3 
		DWORD64 animation_pointer = FindSpecificPointer(hProcess, baseAddress, 3);
		int player_animation = 0;

		//PLAYTIME 4
		DWORD64 playtime_pointer = FindSpecificPointer(hProcess, baseAddress, 4);
		long long playtime = 0;
		std::string playtime_string = "";

		//DEATHNUM 5
		DWORD64 deathnum_pointer = FindSpecificPointer(hProcess, baseAddress, 5);
		int deathnum = 0;
		long long death_time = 0;

		//BOSSTIME 6
		DWORD64 bosstime_pointer = FindSpecificPointer(hProcess, baseAddress, 6);
		unsigned in_a_bossfight = 0;
		unsigned boss_logged = 1;

		unsigned active_fighting = 0;
		std::vector<int> boss_vector_start;
		std::vector<int> boss_vector_end;

		//CURRENT_SOULS 8
		DWORD64 souls_pointer = FindSpecificPointer(hProcess, baseAddress, 8);
		int souls = 0;
		int prev_souls = 0;

		//TOTAL SOULS 9
		DWORD64 total_souls_pointer = FindSpecificPointer(hProcess, baseAddress, 9);
		int total_souls = 0;
		int prev_total_souls = 0;

		//LEVEL 10
		DWORD64 level_pointer = FindSpecificPointer(hProcess, baseAddress, 10);
		int level = 0;

#ifdef DEBUG
		std::cout << std::endl;
		std::cout << "Player's Health Pointer: " << std::hex << health_pointer << std::endl;
		std::cout << "Animation Pointer: " << std::hex << animation_pointer << std::endl;
		std::cout << "Playtime Pointer: " << std::hex << playtime_pointer << std::endl;
		std::cout << "Deathnumber Pointer: " << std::hex << deathnum_pointer << std::endl;
		std::cout << "Bosstime Pointer: " << std::hex << bosstime_pointer << std::endl;
		std::cout << "Current souls Pointer: " << std::hex << deathnum_pointer << std::endl;
		std::cout << "Total souls Pointer: " << std::hex << deathnum_pointer << std::endl;
		std::cout << "Level Pointer: " << std::hex << level_pointer << std::endl;
#endif

		if (health_pointer) {

			if (waiting_for_pointers != 0) {
				waiting_for_pointers = 0;
				std::cout << "\rWaiting for pointers... Pointers found" << std::endl << std::flush;
			}
			while (player_health > 0 && player_health < 2122) {

				//HEALTH
				if (ReadProcessMemory(hProcess, (BYTE*)health_pointer, &player_health, sizeof(player_health), &bytesRead) && bytesRead == sizeof(player_health)) {
#ifdef DEBUG
					std::cout << "Player's Health: " << std::to_string(player_health) << std::endl;
#endif
				}
				else {
					std::cout << "Failed to read player's health!" << std::endl;
					player_health = 0;
				}


				//ANIMATION
				if (ReadProcessMemory(hProcess, (BYTE*)animation_pointer, &player_animation, sizeof(player_animation), &bytesRead) && bytesRead == sizeof(player_animation)) {
#ifdef DEBUG
					std::cout << "Player's Animation: " << player_animation << std::endl;
#endif
				}
				else {
					std::cout << "Failed to read player's animation!" << std::endl;
				}


				//PLAYTIME
				if (ReadProcessMemory(hProcess, (BYTE*)playtime_pointer, &playtime, sizeof(playtime), &bytesRead) && bytesRead == sizeof(playtime)) {
					playtime_string = convertMilliseconds(playtime);
				}
				else {
					std::cout << "Failed to read player's playtime!" << std::endl;
				}


				//BOSSTIME
				if (ReadProcessMemory(hProcess, (BYTE*)bosstime_pointer, &in_a_bossfight, sizeof(in_a_bossfight), &bytesRead) && bytesRead == sizeof(in_a_bossfight)) {
					if (in_a_bossfight == 1) {
#ifdef DEBUG
						std::cout << "Player is in a bossfight!" << std::endl;
#endif
						//If player is in a bossfight and not logged
						if (active_fighting == 0 && boss_logged == 0) {

							active_fighting = 1;

							boss_vector_start = GetBossVector(hProcess, baseAddress);
							std::stringstream log;
							log << "####\n"
								<< "ENTER\n"
								<< "NIL\n"
								<< playtime_string
								<< "\n";
							WriteToLog(Player_ID, log.str());

							std::cout << "Player started a bossfight!\t("
								<< playtime_string << ")"
								<< std::endl;
						}

						if (player_health <= 0 && boss_logged == 0) {

							std::stringstream log1;
							log1 << "####\n"
								<< "DIED\n"
								<< "NIL\n"
								<< playtime_string
								<< "\n";
							WriteToLog(Player_ID, log1.str());

							std::cout << "Player died in a bossfight!\t("
								<< playtime_string
								<< ")"
								<< std::endl;


						}



					}

					//If there is no bossfight
					else {
						boss_logged = 0;

						//If player started a bossfight and logged the start
						if (active_fighting == 1) {
							//Sleep(3000);
							boss_vector_end = GetBossVector(hProcess, baseAddress);
							active_fighting = 0;
							std::string boss_name = CompareBossVectors(boss_vector_start, boss_vector_end);
							if (boss_name != "0") {
								std::stringstream log;
								log << "####\n"
									<< "DEFEATED\n"
									<< boss_name
									<< "\n"
									<< playtime_string
									<< "\n";
								WriteToLog(Player_ID, log.str());

								std::cout << "Player defeated "
									<< boss_name
									<< "!\t("
									<< playtime_string
									<< ")"
									<< std::endl;
							}

						}

#ifdef DEBUG
						std::cout << "Not in a bossfight!" << std::endl;
#endif 
					}
				}
				else {
					std::cout << "Failed to read bosstime!" << std::endl;
				}


				//LEVEL
				if (ReadProcessMemory(hProcess, (BYTE*)level_pointer, &level, sizeof(level), &bytesRead) && bytesRead == sizeof(level)) {
#ifdef DEBUG
					std::cout << "Player's level: " << std::to_string(level) << std::endl;
#endif
				}
				else {
					std::cout << "Failed to read bosstime!" << std::endl;
				}


				//SOUL RECOVERY CHECK
				if (ReadProcessMemory(hProcess, (BYTE*)souls_pointer, &souls, sizeof(souls), &bytesRead) && bytesRead == sizeof(souls)) {
					if (ReadProcessMemory(hProcess, (BYTE*)total_souls_pointer, &total_souls, sizeof(total_souls), &bytesRead) && bytesRead == sizeof(total_souls)) {
#ifdef DEBUG
						std::cout << "Current souls: " << std::to_string(souls) << "\tTotal souls: " << std::to_string(total_souls) << std::endl;
						std::cout << "P.Current souls: " << std::to_string(prev_souls) << "\tP.Total souls: " << std::to_string(prev_total_souls) << std::endl;
#endif

						if (prev_souls == 0 && prev_total_souls == 0) {
						}
						else {
							int diff = souls - prev_souls;
							if (total_souls < diff + prev_total_souls && diff > 0) {
								std::stringstream log;
								log << "###\n"
									<< std::to_string(diff)
									<< "\n"
									<< playtime_string
									<< "\n";
								WriteToLog(Player_ID, log.str());

								std::cout << "Player recovered souls!\t\t("
									<< playtime_string
									<< ")"
									<< std::endl;
							}
						}
						prev_souls = souls;
						prev_total_souls = total_souls;
					}
				}
				else {

					std::cout << "Failed to read souls!" << std::endl;
				}


				//Warping to another bonfire or exiting to menu
				if (player_animation == 68100 || playtime == 0) {
					Sleep(100);
					break;
				}

				//Sitting down at a bonfire
				if (player_animation == 68010 || player_animation == 68011 || player_animation == 68012) {
					if (log_lock == 0) {
						log_lock = 1;
						std::string bonfire_name = BonfirePlace(hProcess, baseAddress);
						std::stringstream log;
						log << "##\n" << bonfire_name << "\n"
							<< std::to_string(level) << "\n"
							<< std::to_string(souls) << "\n"
							<< std::to_string(total_souls) << "\n"
							<< playtime_string
							<< "\n";
						WriteToLog(Player_ID, log.str());


						std::cout << "Player is resting at "
							<< bonfire_name
							<< " bonfire!\t\t("
							<< playtime_string
							<< ")"
							<< std::endl;

					}
				}
				else {
					log_lock = 0;
				}

				//Playtime flag
				if (launchTime == 0) {
					launchTime = 1;
					std::stringstream log;
					log << "#####\n"
						<< playtime_string
						<< "\n";
					WriteToLog(Player_ID, log.str());

					std::cout << "Logging for this session has started !\t\t("
						<< playtime_string << ")"
						<< std::endl;

					std::cout << "Writing logs to: "
						<< Player_ID
						<< std::endl;
				}

#ifdef DEBUG
				std::cout << std::endl;
				//Sleep(1500);
#endif 
				Sleep(1000);

			}
			//Sleep(3000);

			//DEAD
			if (player_health <= 0) {
				if (ReadProcessMemory(hProcess, (BYTE*)deathnum_pointer, &deathnum, sizeof(deathnum), &bytesRead) && bytesRead == sizeof(deathnum)) {

					if (death_time != playtime) {
						death_time = playtime;
						if (deathnum != printed_death) {



							printed_death = deathnum;
							std::stringstream log;
							log << "#\n"
								<< std::to_string(deathnum)
								<< "\n" << playtime_string
								<< "\n";
							WriteToLog(Player_ID, log.str());

							std::cout << "Player died! ["
								<< std::to_string(deathnum)
								<< "]\t\t("
								<< playtime_string
								<< ")"
								<< std::endl;

						}

					}

				}
				else {
					std::cout << "Failed to read deathnum!" << std::endl << std::endl;
				}
				//Sleep(2000);
			}
		}

		else {
			switch (waiting_for_pointers)
			{
			case 0:
				std::cout << "\rWaiting for pointers   ";
				std::cout << "\rWaiting for pointers";
				waiting_for_pointers++;
				break;

			case 1:
				std::cout << ".";
				waiting_for_pointers++;
				break;
			case 2:
				std::cout << ".";
				waiting_for_pointers++;
				break;
			case 3:
				std::cout << ".";
				waiting_for_pointers = 0;
				break;
			}
			Sleep(1000);

		}

	}

	// Close the handle to the process
	CloseHandle(hProcess);


	return 0;
}

