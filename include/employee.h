#pragma once

struct employee {
    int num;        // ID сотрудника
    char name[10];  // Имя
    double hours;   // Часы
};

// Имя канала для связи
// Формат: \\.\pipe\Имя
const char* const PIPE_NAME = "\\\\.\\pipe\\Lab5Pipe";