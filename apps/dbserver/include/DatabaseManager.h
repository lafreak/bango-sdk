#pragma once

#include <bango/network/server.h>
#include <bango/persistence.h>

#include <inix/protocol.h>
#include <inix/common.h>
#include <inix/structures.h>

#include <set>

using namespace bango::network;
using namespace bango::persistence;

#define DISABLE_SECONDARY_PASSWORD

class DatabaseManager
{
public:
    struct GameServer : public writable {
        using writable::writable;
    };

private:
    server<GameServer> m_dbserver;
    pool m_pool;

    std::set<int> m_active_users;

    bool Validate(const std::string& password) const;

public:

    void Initialize();

    void ConnectToPool(const std::string& host, const std::string& port, const std::string& user, const std::string& password, const std::string& schema);
    void StartDBServer(const std::string& host, const std::int32_t port);

    void FlagDisconnected(int id) { m_active_users.erase(id); }

    void SendPlayerList (const std::unique_ptr<GameServer>& s, unsigned int id, int idaccount);
    void SendDeletedList(const std::unique_ptr<GameServer>& s, unsigned int id, int idaccount);

    void Login          (const std::unique_ptr<GameServer>& s, packet& p);
    void SecondaryLogin (const std::unique_ptr<GameServer>& s, packet& p);
    void SecondaryCreate(const std::unique_ptr<GameServer>& s, packet& p);
    void SecondaryChange(const std::unique_ptr<GameServer>& s, packet& p);

    void NewPlayer      (const std::unique_ptr<GameServer>& s, packet& p);
    void DeletePlayer   (const std::unique_ptr<GameServer>& s, packet& p);
    void RestorePlayer  (const std::unique_ptr<GameServer>& s, packet& p);
    void LoadPlayer     (const std::unique_ptr<GameServer>& s, packet& p);
    void LoadItems      (const std::unique_ptr<GameServer>& s, unsigned int id, int idplayer);
};