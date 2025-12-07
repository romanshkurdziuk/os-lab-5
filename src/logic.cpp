#include "logic.h"
#include "core.h" // Нужно для отправки ответов внутри потока
#include <iostream>
#include <fstream>

EmployeeManager::EmployeeManager(const std::string& fname, int count) : filename(fname) {
    employees.resize(count);
    locks.resize(count);

    // Ввод данных
    for (int i = 0; i < count; ++i) {
        employees[i].num = i + 1;
        std::cout << "Employee " << (i + 1) << " -> Name: ";
        std::cin >> employees[i].name;
        std::cout << "Hours: ";
        std::cin >> employees[i].hours;
        InitializeSRWLock(&locks[i]);
    }
    SaveToFile();
}

EmployeeManager::~EmployeeManager() {
    // SRWLock не требуют явного удаления, но вектор очистится сам
}

void EmployeeManager::SaveToFile() {
    std::ofstream outFile(filename, std::ios::binary);
    outFile.write(reinterpret_cast<char*>(employees.data()), employees.size() * sizeof(employee));
    outFile.close();
}

void EmployeeManager::PrintAll() {
    std::cout << "\n--- File Content ---\n";
    for (const auto& e : employees) {
        std::cout << "ID: " << e.num << ", Name: " << e.name << ", Hours: " << e.hours << "\n";
    }
    std::cout << "--------------------\n";
}

int EmployeeManager::FindEmployeeIndex(int id) {
    for (size_t i = 0; i < employees.size(); ++i) {
        if (employees[i].num == id) return (int)i;
    }
    return -1;
}

employee EmployeeManager::StartRead(int index) {
    AcquireSRWLockShared(&locks[index]);
    return employees[index];
}

void EmployeeManager::EndRead(int index) {
    ReleaseSRWLockShared(&locks[index]);
}

employee EmployeeManager::StartModify(int index) {
    AcquireSRWLockExclusive(&locks[index]);
    return employees[index];
}

void EmployeeManager::ApplyModify(int index, const employee& newData) {
    // Обновляем только изменяемые поля, ID оставляем старый (для надежности)
    employees[index] = newData; 
    ReleaseSRWLockExclusive(&locks[index]);
    std::cout << "[Manager] Employee " << employees[index].num << " updated.\n";
}

// --- Логика потока клиента ---

DWORD WINAPI ClientHandlerThread(LPVOID param) {
    ThreadArgs* args = (ThreadArgs*)param;
    HANDLE hPipe = args->hPipe;
    EmployeeManager* mgr = args->manager;
    delete args; // Удаляем структуру аргументов

    Request req;
    while (PipeComm::Receive(hPipe, &req, sizeof(req))) {
        if (req.cmd == EXIT_CMD) break;

        int idx = mgr->FindEmployeeIndex(req.id);

        if (idx == -1) {
            // Сотрудник не найден
            employee err; err.num = -1;
            PipeComm::Send(hPipe, &err, sizeof(err));
            continue;
        }

        if (req.cmd == READ_CMD) {
            // 1. Блокируем и отправляем
            employee data = mgr->StartRead(idx);
            PipeComm::Send(hPipe, &data, sizeof(data));

            // 2. Ждем сигнала об окончании чтения
            char buffer;
            PipeComm::Receive(hPipe, &buffer, 1);

            // 3. Разблокируем
            mgr->EndRead(idx);
        }
        else if (req.cmd == MODIFY_CMD) {
            // 1. Блокируем (эксклюзивно) и отправляем текущие данные
            employee data = mgr->StartModify(idx);
            PipeComm::Send(hPipe, &data, sizeof(data));

            // 2. Ждем новые данные
            employee newData;
            if (PipeComm::Receive(hPipe, &newData, sizeof(newData))) {
                mgr->ApplyModify(idx, newData);
            } else {
                // Если сбой связи, всё равно надо разблокировать!
                mgr->EndRead(idx); // Или ReleaseExclusive, тут надо аккуратно
                // В данном простом примере ApplyModify делает Release
            }
        }
    }

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    return 0;
}