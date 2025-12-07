#include "logic.h"
#include "core.h" // Нужно для отправки ответов внутри потока
#include <iostream>
#include <fstream>

EmployeeManager::EmployeeManager(const std::string& fname) : filename(fname) {}
EmployeeManager::~EmployeeManager() {}

void EmployeeManager::InitializeFromConsole(int count) 
{    
    employees.resize(count);
    locks.resize(count);
    for (int i = 0; i < count; ++i) 
    {
        employees[i].num = i + 1;
        std::cout << "Employee " << (i + 1) << " -> Name: ";
        std::cin >> employees[i].name;
        std::cout << "Hours: ";
        std::cin >> employees[i].hours;
        InitializeSRWLock(&locks[i]);
    }
    SaveToFile();
}

void EmployeeManager::InitializeWithData(const std::vector<employee>& data) 
{
    employees = data;
    locks.resize(employees.size());
    for (size_t i = 0; i < locks.size(); ++i) 
    {
        InitializeSRWLock(&locks[i]);
    }
    SaveToFile();
}

void EmployeeManager::SaveToFile() 
{
    std::ofstream outFile(filename, std::ios::binary);
    outFile.write(reinterpret_cast<char*>(employees.data()), employees.size() * sizeof(employee));
    outFile.close();
}

void EmployeeManager::PrintAll() 
{
    std::cout << "\n--- File Content ---\n";
    for (const auto& e : employees) {
        std::cout << "ID: " << e.num << ", Name: " << e.name << ", Hours: " << e.hours << "\n";
    }
    std::cout << "--------------------\n";
}

int EmployeeManager::FindEmployeeIndex(int id) 
{
    for (size_t i = 0; i < employees.size(); ++i) 
    {
        if (employees[i].num == id) return (int)i;
    }
    return -1;
}

employee EmployeeManager::StartRead(int index) 
{
    AcquireSRWLockShared(&locks[index]);
    return employees[index];
}

void EmployeeManager::EndRead(int index) 
{
    ReleaseSRWLockShared(&locks[index]);
}

employee EmployeeManager::StartModify(int index) 
{
    AcquireSRWLockExclusive(&locks[index]);
    return employees[index];
}

void EmployeeManager::ApplyModify(int index, const employee& newData) 
{
    employees[index] = newData; 
    ReleaseSRWLockExclusive(&locks[index]);
    std::cout << "[Manager] Employee " << employees[index].num << " updated.\n";
}

DWORD WINAPI ClientHandlerThread(LPVOID param) 
{
    ThreadArgs* args = (ThreadArgs*)param;
    HANDLE hPipe = args->hPipe;
    EmployeeManager* mgr = args->manager;
    delete args;

    Request req;
    while (PipeComm::Receive(hPipe, &req, sizeof(req))) 
    {
        if (req.cmd == EXIT_CMD) break;

        int idx = mgr->FindEmployeeIndex(req.id);

        if (idx == -1) 
        {
            employee err; err.num = -1;
            PipeComm::Send(hPipe, &err, sizeof(err));
            continue;
        }

        if (req.cmd == READ_CMD) 
        {
            employee data = mgr->StartRead(idx);
            PipeComm::Send(hPipe, &data, sizeof(data));

            char buffer;
            PipeComm::Receive(hPipe, &buffer, 1);

            mgr->EndRead(idx);
        }
        else if (req.cmd == MODIFY_CMD) 
        {
            employee data = mgr->StartModify(idx);
            PipeComm::Send(hPipe, &data, sizeof(data));

            employee newData;
            if (PipeComm::Receive(hPipe, &newData, sizeof(newData))) 
            {
                mgr->ApplyModify(idx, newData);
            } else 
            {
                mgr->EndRead(idx);
            }
        }
    }

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    return 0;
}