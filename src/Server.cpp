#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>
#include "employee.h"

int main() {
    std::string filename;
    int employeeCount;
    int clientCount; // Переменная для количества клиентов

    // 1. Ввод и создание файла
    std::cout << "Enter binary filename (e.g., data.bin): ";
    std::cin >> filename;

    std::cout << "Enter number of employees: ";
    std::cin >> employeeCount;

    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Error creating file!" << std::endl;
        return 1;
    }

    for (int i = 0; i < employeeCount; ++i) {
        employee emp;
        std::cout << "Employee " << (i + 1) << "\n";
        std::cout << "  ID: "; std::cin >> emp.num;
        std::cout << "  Name: "; std::cin >> emp.name;
        std::cout << "  Hours: "; std::cin >> emp.hours;
        outFile.write(reinterpret_cast<char*>(&emp), sizeof(employee));
    }
    outFile.close();

    // 2. Вывод содержимого (проверка)
    std::cout << "\nFile content:\n";
    std::ifstream inFile(filename, std::ios::binary);
    employee tempEmp;
    while (inFile.read(reinterpret_cast<char*>(&tempEmp), sizeof(employee))) {
        std::cout << "ID: " << tempEmp.num << ", Name: " << tempEmp.name << ", Hours: " << tempEmp.hours << std::endl;
    }
    inFile.close();

    // --- НОВЫЙ КОД: Запуск клиентов ---
    std::cout << "\nEnter number of clients to launch: ";
    std::cin >> clientCount;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    
    // Командная строка: просто имя exe файла
    std::string cmdLine = "Client.exe"; 

    for (int i = 0; i < clientCount; ++i) {
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // Создаем изменяемый буфер для CreateProcess (она не любит const char*)
        std::vector<char> buffer(cmdLine.begin(), cmdLine.end());
        buffer.push_back(0);

        if (CreateProcessA(NULL, &buffer[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            std::cout << "Client " << (i + 1) << " launched.\n";
            // Закрываем дескрипторы, так как управлять процессом мы не будем,
            // они будут общаться с нами через Pipe
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            std::cerr << "Error launching Client " << (i + 1) << "\n";
        }
    }
    // ----------------------------------

    std::cout << "All clients launched. Press Enter to exit server...";
    std::cin.ignore();
    std::cin.get();

    return 0;
}