#include "Socket.h"

Socket& Socket::Get()
{
    static Socket instance;
    return instance;
}

bango::network::client& Socket::DBClient() { return Get().g_dbclient; }
bango::network::server<Player>& Socket::GameServer() { return Get().g_gameserver; }