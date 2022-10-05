#include "DatabaseManager.h"

int main()
{
    DatabaseManager manager;

    manager.Initialize();

    manager.ConnectToPool("localhost", "3301", "root", "pass", "kalonline");
    manager.StartDBServer("localhost", 2999);

    std::cin.get();
    return 0;
}