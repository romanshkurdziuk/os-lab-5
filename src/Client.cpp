#include <iostream>
#include <windows.h>
#include "employee.h"
#include "core.h"

int main() 
{
    HANDLE hPipe;
    
    while (true) 
    {
        hPipe = CreateFileA(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hPipe != INVALID_HANDLE_VALUE) break;
        if (GetLastError() != ERROR_PIPE_BUSY) 
        {
            std::cerr << "Server not found.\n"; Sleep(1000);
        } else 
        {
            WaitNamedPipeA(PIPE_NAME, 2000);
        }
    }
    std::cout << "Connected.\n";

    while (true) 
    {
        std::cout << "\n1. Modify\n2. Read\n3. Exit\n> ";
        int choice; std::cin >> choice;

        if (choice == 3) 
        {
            Request req = { EXIT_CMD, 0 };
            PipeComm::Send(hPipe, &req, sizeof(req));
            break;
        }

        int id;
        std::cout << "Enter ID: "; std::cin >> id;

        Request req = { (choice == 1 ? MODIFY_CMD : READ_CMD), id };
        if (!PipeComm::Send(hPipe, &req, sizeof(req))) break;

        employee emp;
        if (!PipeComm::Receive(hPipe, &emp, sizeof(emp))) break;

        if (emp.num == -1) 
        {
            std::cout << "Not found.\n"; continue;
        }

        std::cout << "Employee: " << emp.name << ", Hours: " << emp.hours << "\n";

        if (choice == 2) 
        {
            std::cout << "Press Enter to release...";
            std::cin.ignore(); std::cin.get();
            char ack = 1;
            PipeComm::Send(hPipe, &ack, 1);
        } 
        else if (choice == 1) 
        {
            std::cout << "New Name: "; std::cin >> emp.name;
            std::cout << "New Hours: "; std::cin >> emp.hours;
            PipeComm::Send(hPipe, &emp, sizeof(emp));
            std::cout << "Updated.\n";
            std::cin.ignore(); std::cin.get();
        }
    }

    CloseHandle(hPipe);
    return 0;
}