#pragma once

#include <bango/network/client.h>
#include <bango/network/server.h>

class Player;
class Socket
{
    bango::network::client g_dbclient;
    bango::network::server<Player> g_gameserver;

    Socket() {}
    ~Socket() {}

    static Socket& Get();

public:
    static bango::network::client& DBClient();
    static bango::network::server<Player>& GameServer();
};