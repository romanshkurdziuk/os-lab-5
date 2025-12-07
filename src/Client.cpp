#include <iostream>
#include <windows.h>
#include "employee.h"

int main() {
    HANDLE hPipe;
    DWORD bytesWritten, bytesRead;

    // 1. Пытаемся подключиться к именованному каналу
    while (true) {
        hPipe = CreateFileA(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hPipe != INVALID_HANDLE_VALUE) break;
        
        if (GetLastError() != ERROR_PIPE_BUSY) {
            std::cerr << "Could not open pipe. Server might not be running.\n";
            Sleep(1000);
            continue;
        }
        // Ждем, если канал занят (хотя у нас UNLIMITED_INSTANCES)
        WaitNamedPipeA(PIPE_NAME, 5000);
    }

    std::cout << "Connected to Server!\n";

    while (true) {
        std::cout << "\nChoose option:\n1. Modify record\n2. Read record\n3. Exit\n> ";
        int choice;
        std::cin >> choice;

        if (choice == 3) {
            Request req = { EXIT_CMD, 0 };
            WriteFile(hPipe, &req, sizeof(req), &bytesWritten, NULL);
            break;
        }

        if (choice != 1 && choice != 2) continue;

        int id;
        std::cout << "Enter Employee ID: ";
        std::cin >> id;

        // Отправляем запрос на сервер
        Request req;
        req.id = id;
        req.cmd = (choice == 1) ? MODIFY_CMD : READ_CMD;

        if (!WriteFile(hPipe, &req, sizeof(req), &bytesWritten, NULL)) {
            std::cerr << "Send failed.\n"; break;
        }

        // Читаем ответ (структуру сотрудника)
        employee emp;
        if (!ReadFile(hPipe, &emp, sizeof(emp), &bytesRead, NULL)) {
            std::cerr << "Read failed.\n"; break;
        }

        if (emp.num == -1) {
            std::cout << "Error: Employee with ID " << id << " not found.\n";
            continue;
        }

        // Вывод данных
        std::cout << "--> ID: " << emp.num << ", Name: " << emp.name << ", Hours: " << emp.hours << "\n";

        if (choice == 2) { // READ
            std::cout << "Press Enter to finish reading (releases lock)...";
            std::cin.ignore(); std::cin.get();
            
            // Сообщаем серверу, что закончили чтение
            char done = 1;
            WriteFile(hPipe, &done, 1, &bytesWritten, NULL);
        }
        else if (choice == 1) { // MODIFY
            std::cout << "Enter new Name: ";
            std::cin >> emp.name;
            std::cout << "Enter new Hours: ";
            std::cin >> emp.hours;

            // Отправляем измененные данные
            WriteFile(hPipe, &emp, sizeof(emp), &bytesWritten, NULL);
            std::cout << "Updated record sent.\n";
            
            std::cout << "Press Enter to finish modification...";
            std::cin.ignore(); std::cin.get();
        }
    }

    CloseHandle(hPipe);
    return 0;
}