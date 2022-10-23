#include "Socket.h"

#include <bango/network/client.h>
#include <bango/network/server.h>

using namespace bango::network;

Socket& Socket::Get()
{
    static Socket instance;
    return instance;
}

class Player;

client& Socket::DBClient() { return Get().g_dbclient; }
server<Player>& Socket::GameServer() { return Get().g_gameserver; }
