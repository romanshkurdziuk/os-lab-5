#pragma once
#include <vector>
#include <string>
#include <windows.h>
#include "employee.h"

class EmployeeManager 
{
private:
    std::vector<employee> employees;
    std::vector<SRWLOCK> locks; 
    std::string filename;

public:
    EmployeeManager(const std::string& fname);
    ~EmployeeManager();

    void InitializeFromConsole(int count);

    void InitializeWithData(const std::vector<employee>& data);

    void SaveToFile();
    void PrintAll();

    int FindEmployeeIndex(int id);

    employee StartRead(int index);
    void EndRead(int index);

    employee StartModify(int index);
    void ApplyModify(int index, const employee& newData);
};

struct ThreadArgs 
{
    HANDLE hPipe;
    EmployeeManager* manager;
};

DWORD WINAPI ClientHandlerThread(LPVOID param);