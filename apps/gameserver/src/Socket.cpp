#include "Socket.h"
#include "Player.h"

using namespace bango::network;

Socket& Socket::Get()
{
    static Socket instance;
    return instance;
}

client& Socket::DBClient() { return Get().g_dbclient; }
server<Player>& Socket::GameServer() { return Get().g_gameserver; }
