#include "GameManager.h"

int main()
{
    GameManager manager;

    manager.Initialize();

    if (!manager.ConnectToDatabase("localhost", 2999))
        return 1;

    if (!manager.StartGameServer("localhost", 3000))
        return 1;

    std::cin.get();
    return 0;
}