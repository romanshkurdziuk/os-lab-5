#include <gtest/gtest.h>
#include <windows.h>
#include <fstream>
#include "logic.h"
#include "core.h"

// --- Тесты Логики (EmployeeManager) ---

class LogicTest : public ::testing::Test {
protected:
    std::string testFile = "test_data.bin";

    void TearDown() override {
        // Удаляем файл после теста
        remove(testFile.c_str());
    }
};

TEST_F(LogicTest, InitializationAndFileCreation) {
    EmployeeManager mgr(testFile);
    
    std::vector<employee> data(2);
    data[0] = {1, "Alice", 10.5};
    data[1] = {2, "Bob", 20.0};

    mgr.InitializeWithData(data);

    // Проверяем, создался ли файл
    std::ifstream f(testFile);
    ASSERT_TRUE(f.good()) << "File should involve created";
    f.close();

    // Проверяем поиск
    EXPECT_EQ(mgr.FindEmployeeIndex(1), 0);
    EXPECT_EQ(mgr.FindEmployeeIndex(2), 1);
    EXPECT_EQ(mgr.FindEmployeeIndex(99), -1);
}

TEST_F(LogicTest, ModificationLogic) {
    EmployeeManager mgr(testFile);
    std::vector<employee> data = {{10, "TestUser", 5.0}};
    mgr.InitializeWithData(data);

    // Имитируем поток, который хочет изменить данные
    int idx = mgr.FindEmployeeIndex(10);
    ASSERT_NE(idx, -1);

    // Блокируем на запись (StartModify)
    employee current = mgr.StartModify(idx);
    EXPECT_STREQ(current.name, "TestUser");

    // Меняем данные
    employee newData = current;
    strcpy(newData.name, "NewName");
    newData.hours = 100.0;

    // Применяем (ApplyModify)
    mgr.ApplyModify(idx, newData);

    // Проверяем через чтение
    employee updated = mgr.StartRead(idx);
    EXPECT_STREQ(updated.name, "NewName");
    EXPECT_DOUBLE_EQ(updated.hours, 100.0);
    mgr.EndRead(idx);
}

// --- Тесты Ядра (PipeComm) ---

TEST(CoreTest, PipeSendReceive) {
    // Для теста Core нам не нужны именованные каналы, 
    // достаточно создать анонимный канал (CreatePipe)
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    
    ASSERT_TRUE(CreatePipe(&hRead, &hWrite, &sa, 0));

    // Данные для отправки
    struct TestData {
        int x;
        char msg[10];
    } sendData = { 123, "Hello" }, recvData;

    // 1. Пишем в канал
    bool sent = PipeComm::Send(hWrite, &sendData, sizeof(sendData));
    ASSERT_TRUE(sent);

    // 2. Читаем из канала
    bool received = PipeComm::Receive(hRead, &recvData, sizeof(recvData));
    ASSERT_TRUE(received);

    // 3. Сверяем
    EXPECT_EQ(recvData.x, 123);
    EXPECT_STREQ(recvData.msg, "Hello");

    CloseHandle(hRead);
    CloseHandle(hWrite);
}