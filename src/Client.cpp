#include <iostream>
#include <windows.h>
#include <string>
#include "employee.h"

int main() {
    HANDLE hPipe;

    std::cout << "Client started. Connecting to server...\n";

    // Пытаемся подключиться к именованному каналу
    while (true) {
        hPipe = CreateFileA(
            PIPE_NAME,              // Имя канала
            GENERIC_READ | GENERIC_WRITE, // Читаем и пишем
            0,                      // Не разделяем доступ
            NULL,                   // Безопасность по умолчанию
            OPEN_EXISTING,          // Открываем существующий
            0,                      // Атрибуты по умолчанию
            NULL                    // Шаблона нет
        );

        // Если подключились успешно - выходим из цикла
        if (hPipe != INVALID_HANDLE_VALUE) {
            break; 
        }

        // Если ошибка не "Pipe Busy", значит всё плохо
        if (GetLastError() != ERROR_PIPE_BUSY) {
            std::cerr << "Could not open pipe. Error code: " << GetLastError() << "\n";
            std::cin.get();
            return 1;
        }

        // Если занято, ждем 20 секунд, вдруг освободится
        if (!WaitNamedPipeA(PIPE_NAME, 20000)) {
            std::cerr << "Could not open pipe: 20 second wait timed out.\n";
            std::cin.get();
            return 1;
        }
    }

    std::cout << "Connected to Server!\n";

    // --- Тут будет цикл отправки команд ---
    // Пока просто повисим, чтобы сервер успел нас заметить
    std::cout << "Working...\n";
    Sleep(2000); 
    // -------------------------------------

    CloseHandle(hPipe);
    std::cout << "Disconnected.\n";
    
    // Пауза перед закрытием окна, чтобы успеть прочитать
    Sleep(1000); 
    return 0;
}