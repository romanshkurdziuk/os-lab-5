#pragma once

struct employee {
    int num;        // ID
    char name[10];  // Имя
    double hours;   // Часы
};

const char* const PIPE_NAME = "\\\\.\\pipe\\TubeLab5Refactored";

enum Command {
    READ_CMD,
    MODIFY_CMD,
    EXIT_CMD
};

struct Request {
    Command cmd;
    int id;
};