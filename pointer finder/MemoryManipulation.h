#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <iomanip> 
#include <sstream> 
#include <psapi.h>
#include <string>
#include <vector>
#include <fstream>
//#define DEBUG

#ifndef MemoryManipulation   
#define MemoryManipulation

/**
 * @brief Function to get the process ID by name
 *
 * @returns Process ID
 */
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

/**
 * @brief Function to get the base address of a module (DarkSoulsIII.exe)
 *
 * @returns Base address
 */
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


/**
 * @brief Finds a specific pointer to a memory address
 *
 * @param process_handle The handle of the process
 * @param base_address THe base address of the process
 * @param initial_offset The initial offset
 * @param offsets offsets that need to be added to base address + initial offset
 * @param offset_amount tha amount of offsets in offsets array
 *
 * @returns
 */
DWORD64 FindPointer(HANDLE process_handle, DWORD64 base_address, int initial_offset, std::vector<DWORD> offsets, int offset_amount) {
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
#ifdef DEBUG
			std::cerr << "Failed to read memory at offset " << std::hex << offsets[i] << std::endl;
#endif
			return 0;
		}
		pointer += offsets[i];  // Apply the offset to get to the next address in the chain
	}

#ifdef DEBUG
	std::cout << std::endl;
#endif


	return pointer;
}

#endif // MemoryManipulation
