#pragma once
#include <vector>
#include <string>
#include <windows.h>
#include "employee.h"

// Класс управляет базой данных сотрудников и синхронизацией
class EmployeeManager {
private:
    std::vector<employee> employees;
    std::vector<SRWLOCK> locks; // Блокировки (по одной на сотрудника)
    std::string filename;

public:
    EmployeeManager(const std::string& fname);
    ~EmployeeManager();

    // 2. Метод для заполнения данными (для Server.cpp)
    void InitializeFromConsole(int count);

    // 3. Метод для заполнения данными (для Тестов)
    void InitializeWithData(const std::vector<employee>& data);

    // Загрузка/Сохранение
    void SaveToFile();
    void PrintAll();

    // Логика обработки запросов
    // Возвращает индекс сотрудника или -1
    int FindEmployeeIndex(int id);

    // Получить данные (Reader lock)
    employee StartRead(int index);
    void EndRead(int index);

    // Получить данные для изменения (Writer lock)
    employee StartModify(int index);
    void ApplyModify(int index, const employee& newData);
};

// Функция потока (вынесена в logic, чтобы Server.cpp был чище)
struct ThreadArgs {
    HANDLE hPipe;
    EmployeeManager* manager;
};

DWORD WINAPI ClientHandlerThread(LPVOID param);