#pragma once

struct employee {
    int num;        // ID сотрудника
    char name[10];  // Имя
    double hours;   // Часы
};

// Имя именованного канала
const char* const PIPE_NAME = "\\\\.\\pipe\\TubeLab5";

// Команды
enum Command {
    READ_CMD,   // Чтение
    MODIFY_CMD, // Изменение
    EXIT_CMD    // Выход
};

// Запрос от клиента к серверу
struct Request {
    Command cmd;
    int id; // ID записи
};