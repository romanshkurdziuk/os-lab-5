#pragma once

struct employee {
    int num;        // ID сотрудника
    char name[10];  // Имя
    double hours;   // Часы
};

// Имя канала
const char* const PIPE_NAME = "\\\\.\\pipe\\Lab5Pipe";

// Типы команд
enum Command {
    READ_CMD,   // Хочу прочитать
    MODIFY_CMD, // Хочу изменить
    EXIT_CMD    // Я ухожу
};

// То, что Клиент отправляет Серверу при первом обращении
struct Request {
    Command cmd;
    int id; // ID сотрудника, с которым хотим работать
};