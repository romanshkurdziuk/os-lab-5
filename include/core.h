#pragma once
#include <windows.h>
#include <string>
#include <vector>

class PipeComm 
{
public:
    static bool Send(HANDLE hPipe, const void* data, size_t size);
    static bool Receive(HANDLE hPipe, void* buffer, size_t size);
};

class ProcessManager 
{
public:
    static void StartClientProcesses(int count, const std::string& exeName);
};