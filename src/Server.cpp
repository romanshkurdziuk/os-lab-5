#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>
#include "employee.h"

// Глобальные переменные для доступа из потоков
std::vector<employee> emps;     // Данные сотрудников в памяти
std::string filename;           // Имя файла
int employeeCount;              // Количество
CRITICAL_SECTION cs;            // Для синхронизации (понадобится позже)

// Функция, которая будет работать в отдельном потоке для каждого клиента
DWORD WINAPI ClientHandler(LPVOID pipe) {
    HANDLE hPipe = (HANDLE)pipe;
    
    // Тут будет логика общения: чтение команд, отправка ответов...
    // Пока просто выведем сообщение
    std::cout << "Client connected! Processing in thread.\n";

    // Имитация работы
    Sleep(1000);

    // Завершение работы с клиентом
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    std::cout << "Client disconnected.\n";
    return 0;
}

int main() {
    int clientCount;

    // 1. Ввод данных (как раньше)
    std::cout << "Enter binary filename (e.g., data.bin): ";
    std::cin >> filename;
    std::cout << "Enter number of employees: ";
    std::cin >> employeeCount;

    // Заполняем вектор и пишем в файл
    emps.resize(employeeCount);
    for (int i = 0; i < employeeCount; ++i) {
        std::cout << "Employee " << (i + 1) << "\n";
        emps[i].num = i + 1; // Авто-ID для простоты, или вводи: std::cin >> emps[i].num;
        std::cout << "  ID: " << emps[i].num << "\n";
        std::cout << "  Name: "; std::cin >> emps[i].name;
        std::cout << "  Hours: "; std::cin >> emps[i].hours;
    }

    // Сохраняем в файл
    std::ofstream outFile(filename, std::ios::binary);
    outFile.write(reinterpret_cast<char*>(emps.data()), employeeCount * sizeof(employee));
    outFile.close();

    // Вывод на экран
    std::cout << "\nData saved. File content:\n";
    for (const auto& e : emps) {
        std::cout << e.num << " " << e.name << " " << e.hours << "\n";
    }

    // 2. Запуск клиентов
    std::cout << "\nEnter number of clients: ";
    std::cin >> clientCount;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    std::string cmdLine = "Client.exe"; 
    std::vector<HANDLE> hThreads; // Храним дескрипторы потоков

    for (int i = 0; i < clientCount; ++i) {
        // А. Запускаем процесс клиента
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        std::vector<char> buffer(cmdLine.begin(), cmdLine.end());
        buffer.push_back(0);

        if (CreateProcessA(NULL, &buffer[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }

        // Б. Создаем канал для этого клиента
        HANDLE hPipe = CreateNamedPipeA(
            PIPE_NAME,                // Имя канала (\\.\pipe\Lab5Pipe)
            PIPE_ACCESS_DUPLEX,       // Двусторонний (чтение и запись)
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // Сообщениями
            PIPE_UNLIMITED_INSTANCES, // Макс. кол-во
            1024, 1024, 0, NULL       // Буферы
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            std::cerr << "Error creating pipe\n";
            continue;
        }

        // В. Ждем, пока созданный клиент подключится к трубе
        // Сервер зависнет тут, пока Client.exe не вызовет CreateFile
        std::cout << "Waiting for Client " << (i + 1) << " to connect...\n";
        bool connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (connected) {
            // Г. Клиент подключился -> запускаем поток для общения с ним
            HANDLE hThread = CreateThread(NULL, 0, ClientHandler, (LPVOID)hPipe, 0, NULL);
            hThreads.push_back(hThread);
        } else {
            CloseHandle(hPipe);
        }
    }

    // Ждем завершения всех потоков обслуживания
    WaitForMultipleObjects(hThreads.size(), hThreads.data(), TRUE, INFINITE);
    
    // Чистим дескрипторы потоков
    for (auto h : hThreads) CloseHandle(h);

    std::cout << "All clients served. Exiting.\n";
    std::cin.ignore();
    std::cin.get();

    return 0;
}