#include "core.h"
#include <iostream>

bool PipeComm::Send(HANDLE hPipe, const void* data, size_t size) 
{
    DWORD bytesWritten;
    return WriteFile(hPipe, data, static_cast<DWORD>(size), &bytesWritten, NULL) != 0;
}

bool PipeComm::Receive(HANDLE hPipe, void* buffer, size_t size) 
{
    DWORD bytesRead;
    return ReadFile(hPipe, buffer, static_cast<DWORD>(size), &bytesRead, NULL) != 0 && bytesRead == size;
}

void ProcessManager::StartClientProcesses(int count, const std::string& exeName) 
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    for (int i = 0; i < count; ++i) 
    {
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        std::string cmd = exeName;
        
        if (!CreateProcessA(NULL, &cmd[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) 
        {
            std::cerr << "Failed to start " << exeName << ". Error: " << GetLastError() << "\n";
        } else 
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }
}