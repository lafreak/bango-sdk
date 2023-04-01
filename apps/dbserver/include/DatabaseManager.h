#pragma once

#include <bango/network/server.h>
#include <bango/persistence.h>

#include <inix/protocol.h>
#include <inix/common.h>
#include <inix/structures.h>

#include <set>
#include <mutex>

using namespace bango::network;
using namespace bango::persistence;

#define DISABLE_SECONDARY_PASSWORD

class DatabaseManager
{
public:
    struct GameServer : public writable, public authorizable {
        using writable::writable;
    };

private:
    server<GameServer> m_dbserver;
    pool m_pool;

    std::set<int> m_active_users;
    std::mutex m_mtx_users;

    bool Validate(const std::string& password) const;

public:

    void Initialize();

    bool ConnectToPool(const std::string& host, const std::string& port, const std::string& user, const std::string& password, const std::string& schema);
    void StartDBServer(const std::string& host, const std::int32_t port);

    void FlagDisconnected(int id) { std::unique_lock<std::mutex> lock(m_mtx_users); m_active_users.erase(id); }

    void SendPlayerList (const std::shared_ptr<GameServer>& s, unsigned int id, int idaccount);
    void SendDeletedList(const std::shared_ptr<GameServer>& s, unsigned int id, int idaccount);

    void Login          (const std::shared_ptr<GameServer>& s, packet& p);
    void SecondaryLogin (const std::shared_ptr<GameServer>& s, packet& p);
    void SecondaryCreate(const std::shared_ptr<GameServer>& s, packet& p);
    void SecondaryChange(const std::shared_ptr<GameServer>& s, packet& p);

    void NewPlayer      (const std::shared_ptr<GameServer>& s, packet& p);
    void DeletePlayer   (const std::shared_ptr<GameServer>& s, packet& p);
    void RestorePlayer  (const std::shared_ptr<GameServer>& s, packet& p);
    void LoadPlayer     (const std::shared_ptr<GameServer>& s, packet& p);
    void LoadItems      (const std::shared_ptr<GameServer>& s, unsigned int id, int idplayer);
    void LoadSkills     (const std::shared_ptr<GameServer>& s, unsigned int id, int idplayer);

    void InsertItem     (const std::shared_ptr<GameServer>& s, packet& p);
    void UpdateItemNum  (const std::shared_ptr<GameServer>& s, packet& p);
    void TrashItem      (const std::shared_ptr<GameServer>& s, packet& p);
    void UpdateItemInfo (const std::shared_ptr<GameServer>& s, packet& p);

    void UpdateProperty (const std::shared_ptr<GameServer>& s, packet& p);
    void SaveAllProperty (const std::shared_ptr<GameServer>& s, packet& p);

    void InsertNewSkill    (const std::shared_ptr<GameServer>& s, packet& p);
    void UpgradeSkill    (const std::shared_ptr<GameServer>& s, packet& p);
};