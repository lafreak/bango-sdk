#pragma once

#include <bango/network.h>

#include <inix/protocol.h>
#include <inix/common.h>
#include <inix/structures.h>

#include "Player.h"

using namespace bango::network;

class GameManager
{
private:
    tcp_server m_gameserver;
    tcp_client m_dbclient;

public:

    void Initialize();
    bool ConnectToDatabase(const std::string& host, const std::int32_t port);
    bool StartGameServer(const std::string& host, const std::int32_t port);
};