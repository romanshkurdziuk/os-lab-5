#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>
#include <algorithm>
#include "employee.h"

// Глобальные данные
std::vector<employee> emps;       // База данных в памяти
std::vector<SRWLOCK> locks;       // Блокировки (по одной на запись)
std::string filename;
int employeeCount;
int clientCount;

// Поток обслуживания клиента
DWORD WINAPI ClientHandler(LPVOID pipe) {
    HANDLE hPipe = (HANDLE)pipe;
    Request req;
    DWORD bytesRead, bytesWritten;

    while (true) {
        // 1. Ждем команду от клиента
        if (!ReadFile(hPipe, &req, sizeof(req), &bytesRead, NULL) || bytesRead == 0) {
            break; // Клиент отключился
        }

        if (req.cmd == EXIT_CMD) {
            break; 
        }

        // 2. Ищем сотрудника по ID
        int index = -1;
        for (size_t i = 0; i < emps.size(); ++i) {
            if (emps[i].num == req.id) {
                index = i;
                break;
            }
        }

        // Если не нашли, отправляем пустую структуру с ID = -1
        if (index == -1) {
            employee errorEmp;
            errorEmp.num = -1;
            WriteFile(hPipe, &errorEmp, sizeof(errorEmp), &bytesWritten, NULL);
            continue;
        }

        // 3. Логика блокировки (Reader/Writer)
        // Мы держим блокировку ПОКА клиент работает с записью (смотрит или меняет)
        
        if (req.cmd == READ_CMD) {
            // ЧИТАТЕЛЬ: Запрашиваем разделяемую блокировку
            AcquireSRWLockShared(&locks[index]);
            
            // Отправляем данные клиенту
            WriteFile(hPipe, &emps[index], sizeof(employee), &bytesWritten, NULL);
            
            // Ждем, пока клиент закончит чтение (он пришлет любой байт подтверждения)
            char buffer;
            ReadFile(hPipe, &buffer, 1, &bytesRead, NULL);
            
            // Снимаем блокировку
            ReleaseSRWLockShared(&locks[index]);
        } 
        else if (req.cmd == MODIFY_CMD) {
            // ПИСАТЕЛЬ: Запрашиваем эксклюзивную блокировку
            AcquireSRWLockExclusive(&locks[index]);
            
            // Отправляем текущие данные
            WriteFile(hPipe, &emps[index], sizeof(employee), &bytesWritten, NULL);
            
            // Ждем от клиента обновленную структуру
            employee newOne;
            if (ReadFile(hPipe, &newOne, sizeof(newOne), &bytesRead, NULL)) {
                // Применяем изменения (ID менять нельзя по логике, но поля обновляем)
                emps[index] = newOne;
                std::cout << "[Server] Employee ID " << req.id << " modified.\n";
            }
            
            // Снимаем блокировку
            ReleaseSRWLockExclusive(&locks[index]);
        }
    }

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    return 0;
}

int main() {
    // 1. Ввод данных
    std::cout << "Enter binary filename: ";
    std::cin >> filename;
    std::cout << "Enter number of employees: ";
    std::cin >> employeeCount;

    emps.resize(employeeCount);
    locks.resize(employeeCount);

    for (int i = 0; i < employeeCount; ++i) {
        emps[i].num = i + 1; // Авто ID для простоты
        std::cout << "Employee ID " << emps[i].num << "\n";
        std::cout << "Name: "; std::cin >> emps[i].name;
        std::cout << "Hours: "; std::cin >> emps[i].hours;
        
        // Инициализируем блокировку для этой записи
        InitializeSRWLock(&locks[i]);
    }

    // Сохраняем в файл (первоначальный)
    std::ofstream outFile(filename, std::ios::binary);
    outFile.write(reinterpret_cast<char*>(emps.data()), employeeCount * sizeof(employee));
    outFile.close();

    // Выводим содержимое
    std::cout << "\nData saved. File content:\n";
    for (const auto& e : emps) {
        std::cout << e.num << " " << e.name << " " << e.hours << "\n";
    }

    // 2. Запуск клиентов
    std::cout << "\nEnter number of clients: ";
    std::cin >> clientCount;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    // Client.exe должен лежать рядом с Server.exe
    std::string cmdLine = "Client.exe"; 
    
    for (int i = 0; i < clientCount; ++i) {
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        // Создаем новое консольное окно для клиента
        if (CreateProcessA(NULL, const_cast<char*>(cmdLine.c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            std::cerr << "Failed to launch Client.exe (Error " << GetLastError() << ")\n";
        }
    }

    // 3. Создаем каналы и ждем подключений
    std::vector<HANDLE> hThreads;
    for (int i = 0; i < clientCount; ++i) {
        HANDLE hPipe = CreateNamedPipeA(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            1024, 1024, 0, NULL
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            std::cerr << "CreateNamedPipe failed\n";
            continue;
        }

        // Ждем подключения клиента
        bool connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        
        if (connected) {
            // Запускаем поток для работы с клиентом
            HANDLE hThread = CreateThread(NULL, 0, ClientHandler, (LPVOID)hPipe, 0, NULL);
            hThreads.push_back(hThread);
        } else {
            CloseHandle(hPipe);
        }
    }

    // Ждем завершения всех потоков (клиентов)
    WaitForMultipleObjects(hThreads.size(), hThreads.data(), TRUE, INFINITE);
    for (auto h : hThreads) CloseHandle(h);

    // 4. Финальный вывод в файл и консоль
    std::cout << "\nAll clients finished. Final file content:\n";
    std::ofstream finalFile(filename, std::ios::binary);
    finalFile.write(reinterpret_cast<char*>(emps.data()), employeeCount * sizeof(employee));
    finalFile.close();

    for (const auto& e : emps) {
        std::cout << e.num << " " << e.name << " " << e.hours << "\n";
    }

    std::cout << "Press Enter to exit...";
    std::cin.ignore(); std::cin.get();

    return 0;
}