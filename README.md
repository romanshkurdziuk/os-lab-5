# 🔄 IPC Named Pipes & Readers-Writers Synchronization

![C++](https://img.shields.io/badge/C++-17-blue.svg?style=flat&logo=c%2B%2B)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6.svg?style=flat&logo=windows)
![Build](https://img.shields.io/badge/Build-CMake-064F8C.svg?style=flat&logo=cmake)
![Tests](https://img.shields.io/badge/Tests-GoogleTest-green.svg?style=flat&logo=google)

A robust implementation of **Client-Server Architecture** using **Named Pipes** for Inter-Process Communication (IPC). The system solves the classical **Readers-Writers Problem** using Windows API `SRWLock` primitives to manage concurrent access to a shared database of employees.

---

## 🚀 Key Features

*   **Architecture:** Modular design separating **Core** (IPC wrappers), **Logic** (Business rules), and **UI**.
*   **Communication:** High-performance data transfer using Windows **Named Pipes**.
*   **Synchronization:** Implementation of the **Readers-Writers** algorithm using `SRWLock` (Slim Reader/Writer Locks).
    *   Multiple clients can **read** simultaneously.
    *   **Writing** blocks all other readers and writers.
*   **Quality Assurance:** Fully covered by **Unit Tests** using the **Google Test** framework.
*   **Build System:** Modern **CMake** configuration with automatic dependency fetching.

---

## 📂 Project Structure

```text
Lab5_Pipes/
├── CMakeLists.txt       # Build configuration & GTest setup
├── include/             # Public interface
│   ├── core.h           # Pipe communication wrapper & Process mgmt
│   ├── logic.h          # Employee Database Manager & Locking logic
│   └── employee.h       # Shared Protocol (Structures & Constants)
├── src/                 # Implementation
│   ├── core.cpp         # Low-level Windows API wrapper
│   ├── logic.cpp        # Business logic implementation
│   ├── Server.cpp       # Server process (Entry point)
│   └── Client.cpp       # Client process (Entry point)
└── tests/               # Automated testing
    └── test_main.cpp    # Google Test suite for Logic and Core
```

---

## 🛠️ How it Works

The system consists of a central **Server** managing a file database and multiple **Client** processes connecting via `\\.\pipe\TubeLab5Refactored`.

### 1. Communication Protocol
Clients send a `Request` structure containing a command and an ID. The Server responds with `employee` data.

| Command | Description |
| :--- | :--- |
| `READ_CMD` | Requests shared access to read record details. |
| `MODIFY_CMD` | Requests exclusive access to update record details. |
| `EXIT_CMD` | Notifies the server of disconnection. |

### 2. Synchronization Logic (SRWLock)
Instead of simple Mutexes, this project uses **Slim Reader/Writer Locks** for efficiency:

*   **Reading (`AcquireSRWLockShared`):**
    *   Multiple clients can read the same record ID at the same time.
    *   Access is denied only if a Writer is currently modifying that specific record.
*   **Modifying (`AcquireSRWLockExclusive`):**
    *   When a client wants to modify, it gains exclusive access.
    *   All other Readers and Writers for that ID are blocked until the modification is complete.

---

## ⚙️ Building the Project

Ensure you have **CMake**, a C++ Compiler (MSVC/MinGW), and Internet access (to fetch GTest).

1.  Clone the repository:
    ```bash
    git clone https://github.com/YourUsername/Lab5_Pipes.git
    cd Lab5_Pipes
    ```

2.  Create a build directory:
    ```bash
    mkdir build
    cd build
    ```

3.  Compile (CMake will automatically download Google Test):
    ```bash
    cmake ..
    cmake --build .
    ```

---

## 🎮 Usage

### 1. Start the Server
The server initializes the binary file and waits for connections.
```powershell
./Server.exe
# Follow prompts:
# Enter filename: data.bin
# Enter employee count: 3
# ... (Fill initial data) ...
# Enter client count: 2
```

### 2. Client Interaction
The Server automatically launches `Client.exe` instances. In the client console:

*   **Option 1 (Modify):** Enter ID. The server locks the record exclusively. You see current data, enter new data, and the lock is released.
*   **Option 2 (Read):** Enter ID. You see current data. The lock is held (shared) until you press Enter. Other clients can also read this ID, but cannot modify it until you finish.

---

## 🧪 Running Tests

The project includes a comprehensive suite of Unit Tests ensuring the logic works without manual input.

Run the test executable from the build folder:
```powershell
./UnitTests.exe
# Or use CTest:
ctest -C Debug --output-on-failure
```

**Expected Output:**
```text
[==========] Running 3 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 2 tests from LogicTest
[ RUN      ] LogicTest.InitializationAndFileCreation
[       OK ] LogicTest.InitializationAndFileCreation (2 ms)
[ RUN      ] LogicTest.ModificationLogic
[       OK ] LogicTest.ModificationLogic (1 ms)
[----------] 2 tests from LogicTest (5 ms total)
[----------] 1 test from CoreTest
[ RUN      ] CoreTest.PipeSendReceive
[       OK ] CoreTest.PipeSendReceive (0 ms)
[==========] 3 tests from 2 test suites ran.
[  PASSED  ] 3 tests.
```

---

## 👨‍💻 Author

Student of BSU FAMCS
Project for "Operating Systems" Course (Lab 5).