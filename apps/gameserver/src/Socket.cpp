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

Player* Socket::FindUserByUID(unsigned int uid)
{
    for (const auto& s : GameServer().sessions())
        if (s.second->GetUID() == uid)
            return s.second.get();

    return nullptr;
}