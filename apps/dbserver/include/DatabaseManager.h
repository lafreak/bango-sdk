#pragma once

#include <bango/network.h>
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
private:
    tcp_server m_dbserver;
    pool m_pool;

    std::set<int> m_active_users;

    bool Validate(const std::string& password) const;

public:
    void Initialize();

    bool ConnectToPool(const std::string& host, const std::string& port, const std::string& user, const std::string& password, const std::string& schema);
    bool StartDBServer(const std::string& host, const std::int32_t port);

    void FlagDisconnected(int id) { m_active_users.erase(id); }

    void SendPlayerList (const std::shared_ptr<session>& s, unsigned int id, int idaccount);
    void SendDeletedList(const std::shared_ptr<session>& s, unsigned int id, int idaccount);

    void Login          (const std::shared_ptr<session>& s, packet& p);
    void SecondaryLogin (const std::shared_ptr<session>& s, packet& p);
    void SecondaryCreate(const std::shared_ptr<session>& s, packet& p);
    void SecondaryChange(const std::shared_ptr<session>& s, packet& p);

    void NewPlayer      (const std::shared_ptr<session>& s, packet& p);
    void DeletePlayer   (const std::shared_ptr<session>& s, packet& p);
    void RestorePlayer  (const std::shared_ptr<session>& s, packet& p);
    void LoadPlayer     (const std::shared_ptr<session>& s, packet& p);
    void LoadItems      (const std::shared_ptr<session>& s, unsigned int id, int idplayer);
};