#include "DatabaseManager.h"

int main()
{
    DatabaseManager manager;

    manager.Initialize();

    if (!manager.ConnectToPool("localhost", "3301", "root", ";", "kalonline"))
        return 1;

    if (!manager.StartDBServer("localhost", 2999))
        return 1;

    std::cin.get();
    return 0;
}