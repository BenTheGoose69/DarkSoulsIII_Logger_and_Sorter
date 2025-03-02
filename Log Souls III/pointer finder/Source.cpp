#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <tlhelp32.h>

// Function to read memory
bool ReadMemory(HANDLE hProcess, LPCVOID baseAddress, void* buffer, SIZE_T size) {
    SIZE_T bytesRead;
    if (!ReadProcessMemory(hProcess, baseAddress, buffer, size, &bytesRead)) {
        std::cerr << "ReadProcessMemory failed. Error: " << GetLastError() << std::endl;
        return false;
    }
    return bytesRead == size;
}

// Function to get the process ID by name
DWORD GetProcessID(const wchar_t* processName) {
    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateToolhelp32Snapshot failed. Error: " << GetLastError() << std::endl;
        return processID;
    }

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
    else {
        std::cerr << "Process32FirstW failed. Error: " << GetLastError() << std::endl;
    }
    CloseHandle(hProcessSnap);
    return processID;
}

// Function to scan memory for an AOB pattern
std::vector<DWORD64> ScanMemoryRegionForAOB(HANDLE hProcess, DWORD64 startAddress, DWORD64 endAddress, const std::string& aobPattern) {
    std::vector<DWORD64> addresses;
    std::vector<BYTE> pattern;
    std::vector<BYTE> mask;

    // Parse AOB pattern
    std::istringstream patternStream(aobPattern);
    std::string byte;
    while (std::getline(patternStream, byte, ' ')) {
        if (byte == "??") {
            mask.push_back(0x00);
            pattern.push_back(0x00);
        }
        else {
            mask.push_back(0xFF);
            pattern.push_back(static_cast<BYTE>(std::stoi(byte, nullptr, 16)));
        }
    }

    DWORD64 currentAddress = startAddress;
    while (currentAddress < endAddress) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQueryEx(hProcess, (LPCVOID)currentAddress, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            // Only scan regions that are readable
            if (mbi.Protect & PAGE_READONLY || mbi.Protect & PAGE_READWRITE) {
                DWORD64 regionEnd = currentAddress + mbi.RegionSize;
                if (regionEnd > endAddress) {
                    regionEnd = endAddress;
                }

                // Scan the region
                while (currentAddress < regionEnd) {
                    std::vector<BYTE> buffer(pattern.size());
                    if (ReadMemory(hProcess, reinterpret_cast<LPCVOID>(currentAddress), buffer.data(), buffer.size())) {
                        bool found = true;
                        for (size_t i = 0; i < pattern.size(); ++i) {
                            if ((buffer[i] & mask[i]) != pattern[i]) {
                                found = false;
                                break;
                            }
                        }
                        if (found) {
                            addresses.push_back(currentAddress);
                        }
                    }
                    else {
                        std::cerr << "Failed to read memory at address: " << std::hex << currentAddress << std::endl;
                    }
                    currentAddress += buffer.size(); // Move to the next address range
                }
            }
            currentAddress = regionEnd; // Skip to the next region
        }
        else {
            std::cerr << "VirtualQueryEx failed at address: " << std::hex << currentAddress << ". Error: " << GetLastError() << std::endl;
            break;
        }
    }

    return addresses;
}

int main() {
    const wchar_t* processName = L"DarkSoulsIII.exe";
    const std::string aobPattern = "48 8B 05 ?? ?? ?? ?? 48 85 C0"; // Example pattern

    DWORD processID = GetProcessID(processName);
    if (processID) {
        std::wcout << L"Process ID: " << processID << std::endl;

        HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processID);
        if (hProcess) {
            SYSTEM_INFO sysInfo;
            GetSystemInfo(&sysInfo);

            DWORD64 startAddress = reinterpret_cast<DWORD64>(sysInfo.lpMinimumApplicationAddress);
            DWORD64 endAddress = reinterpret_cast<DWORD64>(sysInfo.lpMaximumApplicationAddress);

            std::cout << "Scanning memory from: " << std::hex << startAddress << " to " << std::hex << endAddress << std::endl;

            std::vector<DWORD64> foundAddresses = ScanMemoryRegionForAOB(hProcess, startAddress, endAddress, aobPattern);
            if (!foundAddresses.empty()) {
                for (DWORD64 address : foundAddresses) {
                    std::cout << "Found address: " << std::hex << address << std::endl;
                }
            }
            else {
                std::cerr << "No addresses found with the AOB pattern!" << std::endl;
            }

            CloseHandle(hProcess);
        }
        else {
            std::cerr << "Failed to open process. Error: " << GetLastError() << std::endl;
        }
    }
    else {
        std::cerr << "Failed to find process ID!" << std::endl;
    }

    return 0;
}
