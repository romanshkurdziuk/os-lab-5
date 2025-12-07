#include <iostream>
#include <vector>
#include <windows.h>
#include "employee.h"
#include "core.h"
#include "logic.h"

int main() {
    std::string filename;
    int empCount, clientCount;

    std::cout << "Enter filename: "; std::cin >> filename;
    std::cout << "Enter employee count: "; std::cin >> empCount;

    // Инициализация менеджера (внутри будет ввод данных)
    EmployeeManager manager(filename, empCount);
    manager.PrintAll();

    std::cout << "Enter client count: "; std::cin >> clientCount;
    
    // Запуск процессов
    ProcessManager::StartClientProcesses(clientCount, "Client.exe");

    // Создание каналов
    std::vector<HANDLE> threads;
    for (int i = 0; i < clientCount; ++i) {
        HANDLE hPipe = CreateNamedPipeA(
            PIPE_NAME, PIPE_ACCESS_DUPLEX, 
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES, 1024, 1024, 0, NULL
        );

        if (hPipe == INVALID_HANDLE_VALUE) continue;

        // Ждем подключения
        bool connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (connected) {
            // Передаем аргументы в поток
            ThreadArgs* args = new ThreadArgs{ hPipe, &manager };
            HANDLE hThread = CreateThread(NULL, 0, ClientHandlerThread, args, 0, NULL);
            threads.push_back(hThread);
        } else {
            CloseHandle(hPipe);
        }
    }

    WaitForMultipleObjects(threads.size(), threads.data(), TRUE, INFINITE);
    for (auto h : threads) CloseHandle(h);

    std::cout << "All clients finished.\n";
    manager.PrintAll();
    manager.SaveToFile();

    system("pause");
    return 0;
}