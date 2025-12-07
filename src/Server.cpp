#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h> // Понадобится позже для Named Pipes
#include "employee.h"

int main() {
    std::string filename;
    int employeeCount;

    // 1. Ввод имени файла
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
        std::cout << "  ID: "; 
        std::cin >> emp.num;
        std::cout << "  Name (max 9 chars): "; 
        std::cin >> emp.name;
        std::cout << "  Hours: "; 
        std::cin >> emp.hours;

        // Записываем структуру целиком в бинарном виде
        outFile.write(reinterpret_cast<char*>(&emp), sizeof(employee));
    }
    outFile.close();

    // 3. Вывод содержимого файла (Проверка)
    std::cout << "\nFile content:\n";
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error opening file for reading!" << std::endl;
        return 1;
    }

    employee tempEmp;
    // Читаем пока читается
    while (inFile.read(reinterpret_cast<char*>(&tempEmp), sizeof(employee))) {
        std::cout << "ID: " << tempEmp.num 
                  << ", Name: " << tempEmp.name 
                  << ", Hours: " << tempEmp.hours << std::endl;
    }
    inFile.close();

    std::cout << "\nServer data initialized. Press Enter to continue setup...";
    std::cin.ignore();
    std::cin.get();

    return 0;
}