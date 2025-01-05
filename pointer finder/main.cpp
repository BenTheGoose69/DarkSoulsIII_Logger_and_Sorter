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
	const wchar_t* process_name = L"DarkSoulsIII.exe";
	const wchar_t* module_name = L"DarkSoulsIII.exe";



	// ProcessID (PID) of the game
	DWORD process_ID = GetProcessID(process_name);
	if (process_ID) {

		DWORD64 baseAddress = GetModuleBaseAddress(process_ID, module_name);
		if (baseAddress) {

			// Open the process with read permissions
			HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, process_ID);
			if (hProcess) {


				SIZE_T bytesRead;


				int printed_death = 0;
				int waiting = 0;

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
					unsigned boss_log = 0;

					unsigned active_fighting = 0;
					std::vector<int> boss_vector_start;
					std::vector<int> boss_vector_end;

					//CURRENT_SOULS 8
					DWORD64 souls_pointer = FindSpecificPointer(hProcess, baseAddress, 8);
					int souls;
					int prev_souls = 0;

					//TOTAL SOULS 9
					DWORD64 total_souls_pointer = FindSpecificPointer(hProcess, baseAddress, 9);
					int total_souls;
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
						if (waiting == 1) {
							waiting = 0;
							std::cout << "Pointers found!" << std::endl;
						}
						while (player_health != 0) {

							

							//HEALTH
							if (ReadProcessMemory(hProcess, (BYTE*)health_pointer, &player_health, sizeof(player_health), &bytesRead) && bytesRead == sizeof(player_health)) {
#ifdef DEBUG
								std::cout << "Player's Health: " << std::to_string(player_health) << std::endl;
#endif
							}
							else {
								std::cerr << "Failed to read player's health!" << std::endl;
								player_health = 0;
							}


							//ANIMATION
							if (ReadProcessMemory(hProcess, (BYTE*)animation_pointer, &player_animation, sizeof(player_animation), &bytesRead) && bytesRead == sizeof(player_animation)) {
#ifdef DEBUG
								std::cout << "Player's Animation: " << player_animation << std::endl;
#endif
							}
							else {
								std::cerr << "Failed to read player's animation!" << std::endl;
							}


							//PLAYTIME
							if (ReadProcessMemory(hProcess, (BYTE*)playtime_pointer, &playtime, sizeof(playtime), &bytesRead) && bytesRead == sizeof(playtime)) {
								playtime_string = convertMilliseconds(playtime);
							}
							else {
								std::cerr << "Failed to read player's playtime!" << std::endl;
							}

							//BOSSTIME
							if (ReadProcessMemory(hProcess, (BYTE*)bosstime_pointer, &in_a_bossfight, sizeof(in_a_bossfight), &bytesRead) && bytesRead == sizeof(in_a_bossfight)) {
								if (in_a_bossfight == 1) {
#ifdef DEBUG
									std::cout << "Player is in a bossfight!" << std::endl;
#endif

									//If player is in a bossfight and not logged
									if (active_fighting == 0) {
										std::stringstream log;
										active_fighting = 1;
										boss_vector_start = GetBossVector(hProcess, baseAddress);
										std::cout << "Player started a bossfight!\t(" << playtime_string << ")" << std::endl;
										log << "####\n" << "ENTER\n" << "NIL\n" << playtime_string << "\n";
										WriteToLog(Player_ID, log.str());
									}
									if (active_fighting == 1 && player_health == 0) {
										std::stringstream log;
										std::cout << "Player died in a bossfight!\t(" << playtime_string << ")" << std::endl;
										log << "####\n" << "DIED\n" << "NIL\n" << playtime_string << "\n";
										WriteToLog(Player_ID, log.str());
									}

								}
								else {
									if (active_fighting == 1) {
										boss_vector_end = GetBossVector(hProcess, baseAddress);
										active_fighting = 0;
										std::string boss_name = CompareBossVectors(boss_vector_start, boss_vector_end);
										if (boss_name != "0") {
											std::stringstream log;
											std::cout << "Player defeated " << boss_name << "!\t(" << playtime_string << ")" << std::endl;
											log << "####\n" << "DEFEATED\n" << boss_name << "\n" << playtime_string << "\n";
											WriteToLog(Player_ID, log.str());
										}
									}
#ifdef DEBUG
									std::cout << "Not in a bossfight!" << std::endl;
#endif 
								}
							}
							else {
								std::cerr << "Failed to read bosstime!" << std::endl;
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
											std::cout << "Player recovered souls!\t\t(" << playtime_string << ")" << std::endl;
											log << "###\n" << std::to_string(diff) << "\n" << playtime_string << "\n";
											WriteToLog(Player_ID, log.str());
										}
									}
									prev_souls = souls;
									prev_total_souls = total_souls;
								}
							}
							else {

								std::cerr << "Failed to read souls!" << std::endl;
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
										<< playtime_string << "\n";
									WriteToLog(Player_ID, log.str());

									std::cout << "Player is resting at " << bonfire_name << " bonfire!\t\t(" << playtime_string << ")" << std::endl;

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
									<< playtime_string << "\n";
								WriteToLog(Player_ID, log.str());

								std::cout << "Logging for this session has started !\t\t(" << playtime_string << ")" << std::endl;
							}

#ifdef DEBUG
							std::cout << std::endl;
							//Sleep(1500);
#endif 
							Sleep(1000);

						}
						Sleep(3000);

						//DEAD
						if (player_health == 0) {
							if (ReadProcessMemory(hProcess, (BYTE*)deathnum_pointer, &deathnum, sizeof(deathnum), &bytesRead) && bytesRead == sizeof(deathnum)) {
								if (death_time != playtime) {
									death_time = playtime;
									if (deathnum != printed_death) {
										std::stringstream log;
										std::cout << "Player died! [" << std::to_string(deathnum) << "]\t\t(" << playtime_string << ")" << std::endl;
										printed_death = deathnum;
										log << "#\n" << std::to_string(deathnum) << "\n" << playtime_string << "\n";
										WriteToLog(Player_ID, log.str());
									}
								}

							}
							else {
								std::cout << "Failed to read deathnum!" << std::endl << std::endl;
							}
							Sleep(1500);
						}
					}

					else {
						switch (waiting)
						{
						case 0:
							std::cerr << "Waiting for pointers...";
							waiting = 1;
							break;

						case 1:
							std::cerr << ".";
							break;
						}
						
						Sleep(1000);
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
