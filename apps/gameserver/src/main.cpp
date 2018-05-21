#include <bango/network/client.h>
#include <bango/network/server.h>

#include <bango/space/quadtree.h>

#include <inix/protocol.h>
#include <inix/common.h>
#include <inix/structures.h>

#include <iostream>
#include <cassert>

using namespace bango::network;
using namespace bango::space;

class User : public writable
{
    bool m_in_game=false;

    unsigned int    m_uid;
    int             m_aid;
public:
    explicit User(const taco_client_t& client) : writable(client)
    {
        static unsigned int g_max_uid=0;
        m_uid = g_max_uid++;
    }

    bool InGame() const { return m_in_game; }

    void InitPlayer()       { m_in_game = true; } 
    void DestroyPlayer()  { m_in_game = false; }

    unsigned int    GetUID() const { return m_uid; }
    int             GetAID() const { return m_aid; }

    void            SetAID(int value)   { m_aid = value; }
};

class Player : public quad_entity, public User
{
    PLAYERINFO m_data;
    std::string m_name;
public:
    using User::User;

    void OnLoadPlayer(packet& p)
    {
        p >> m_data >> m_name;
        m_x = p.pop<int>();
        m_y = p.pop<int>();

        write(S2C_PROPERTY, "bsbwwwwwwddwwwwwbIwwwwwwbbbbbd",
            0, //Grade
            "\0", //GuildName
            0, //GRole
            GetContribute(),
            GetStrength(),
            GetHealth(),
            GetInteligence(),
            GetWisdom(),
            GetDexterity(),
            GetCurHP(),             //!
            GetCurHP(), //MaxHP     //!
            GetCurMP(),
            GetCurMP(), //MaxMP
            1, //Hit
            2, //Dodge
            3, //Defense            //!
            4, //Absorb             //!
            GetExp(),               //!
            5, //MinAttack
            6, //MaxAttack
            7, //MinMagic
            8, //MaxMagic
            GetPUPoint(),
            GetSUPoint(),           //!
            9, //ResistFire
            10, //ResistIce
            11, //ResistLitning
            12, //ResistCurse
            13, //ResistPalsy
            GetRage());

        short time = 1200;
        write(S2C_ANS_LOAD, "wdd", time, GetX(), GetY());
    }

    void OnLoadItems(packet& p)
    {
    }

    void GameStart(packet& p)
    {
        auto unknown = p.pop<char>();
        auto height = p.pop<int>();

        // Map thing
        write(S2C_CREATEPLAYER, "dsbdddwIwwwwwwwwbbIssdbdddIIbddb",
            GetUID(), // CharacterID!
            GetName().c_str(),
            GetClass() | GAME_HERO,
            GetX(),
            GetY(),
            GetZ(),
            0, //Dir
            0, //GState
            0, //GI Weapon
            0, //GI Shield
            0, //GI Helmet
            0, //GI Upper
            0, //GI Lower
            0, //GI Gauntlet
            0, //GI Boots
            0, //GI Costume
            GetFace(),
            GetHair(),
            0, //MState
            "\0", //GuildClass
            "\0", //GuildName
            0, //GID
            0, //Flag
            0, //FlagItem
            0, //HonorGrade
            0, //HonorOption
            0, //GStateEx
            0, //MStateEx

            0, //Unk
            0, //Unk
            0, //Unk
            0  //Unk
            );
    }

    void GameRestart() {}

    bool CanLogout() const { return true; }

    const std::string&  GetName()       const { return m_name; }
    unsigned char       GetClass()      const { return m_data.Class; }
    unsigned char       GetJob()        const { return m_data.Job; }
    unsigned char       GetLevel()      const { return m_data.Level; }
    unsigned short      GetStrength()   const { return m_data.Strength; }
    unsigned short      GetHealth()     const { return m_data.Health; }
    unsigned short      GetInteligence()const { return m_data.Inteligence; }
    unsigned short      GetWisdom()     const { return m_data.Wisdom; }
    unsigned short      GetDexterity()  const { return m_data.Dexterity; }
    unsigned int        GetCurHP()      const { return m_data.CurHP; }
    unsigned int        GetCurMP()      const { return m_data.CurMP; }
    unsigned long       GetExp()        const { return m_data.Exp; }
    unsigned short      GetPUPoint()    const { return m_data.PUPoint; }
    unsigned short      GetSUPoint()    const { return m_data.SUPoint; }
    unsigned short      GetContribute() const { return m_data.Contribute; }
    unsigned int        GetRage()       const { return m_data.Rage; }
    unsigned int        GetX()          const { return m_x; }
    unsigned int        GetY()          const { return m_y; }
    unsigned int        GetZ()          const { return m_data.Z; }
    unsigned char       GetFace()       const { return m_data.Face; }
    unsigned char       GetHair()       const { return m_data.Hair; }
};

class GameManager
{
    client          m_dbclient;
    server<Player>  m_gameserver;

    // BUG: Very inefficient.
    void UserByUID(unsigned int uid, const std::function<void(const std::unique_ptr<Player>&)>&& callback) const
    {
        for (auto& session : m_gameserver.sessions())
        {
            if (session.second->GetUID() == uid)
                callback(session.second);
        }
    }

public:
    void ConnectToDatabase  (const std::string& host, const std::int32_t port) { m_dbclient.connect(host, port); }
    void StartGameServer    (const std::string& host, const std::int32_t port) { m_gameserver.start(host, port); }

    GameManager()
    {
        m_dbclient.when(D2S_LOGIN, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::unique_ptr<Player>& user) {
                user->write(p.change_type(S2C_ANS_LOGIN));
            });
        });

        m_dbclient.when(D2S_AUTHORIZED, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::unique_ptr<Player>& user) {
                user->SetAID(p.pop<int>());
            });
        });

        m_dbclient.when(D2S_SEC_LOGIN, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::unique_ptr<Player>& user) {
                user->write(p.change_type(S2C_SECOND_LOGIN));
            });
        });

        m_dbclient.when(D2S_PLAYER_INFO, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::unique_ptr<Player>& user) {
                user->write(p.change_type(S2C_PLAYERINFO));
            });
        });

        m_dbclient.when(D2S_DELPLAYERINFO, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::unique_ptr<Player>& user) {
                user->write(p.change_type(S2C_DELPLAYERINFO));
            });
        });
        
        m_dbclient.when(D2S_ANS_NEWPLAYER, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::unique_ptr<Player>& user) {
                user->write(p.change_type(S2C_ANS_NEWPLAYER));
            });
        });

        m_dbclient.when(D2S_LOADPLAYER, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::unique_ptr<Player>& user) {
                if (p.pop<char>())
                {
                    user->write(S2C_MESSAGE, "b", MSG_NOTEXISTPLAYER);
                    return;
                }

                // BUG: Player might log out by the time packet arrived.
                assert(!user->InGame());
                user->InitPlayer();
                user->OnLoadPlayer(p);
            });
        });

        m_dbclient.when(D2S_LOADITEMS, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::unique_ptr<Player>& user) {
                user->OnLoadItems(p);
            });
        });

        m_gameserver.when(C2S_CONNECT, [&](const std::unique_ptr<Player>& user, packet& p) {
            if (user->InGame()) return;
            user->write(S2C_CODE, "dbdddIbbb", 0, 0, 604800, 0, 0, 0, 0, 0, 2);
        });

        m_gameserver.when(C2S_ANS_CODE, [&](const std::unique_ptr<Player>& user, packet& p) {
            // ignore...
        });

        m_gameserver.when(C2S_LOGIN, [&](const std::unique_ptr<Player>& user, packet& p) {
            if (user->InGame()) return;
            p.push<unsigned int>(user->GetUID());
            m_dbclient.write(p.change_type(S2D_LOGIN));
        });

        m_gameserver.when(C2S_SECOND_LOGIN, [&](const std::unique_ptr<Player>& user, packet& p) {
            if (user->InGame()) return;
            p.push<int>(user->GetAID());
            p.push<unsigned int>(user->GetUID());
            m_dbclient.write(p.change_type(S2D_SECONDARY_LOGIN));
        });

        m_gameserver.when(C2S_NEWPLAYER, [&](const std::unique_ptr<Player>& user, packet& p) {
            if (user->InGame()) return;

            packet copy = p;
            
            auto name = p.pop_str();
            auto job = p.pop<unsigned char>();
            auto str = p.pop<unsigned short>();
            auto hth = p.pop<unsigned short>();
            auto int_ = p.pop<unsigned short>();
            auto wis = p.pop<unsigned short>();
            auto dex = p.pop<unsigned short>();
            auto face = p.pop<unsigned char>();
            auto hair = p.pop<unsigned char>();

            if (name.empty() || name.size() > 14)
            {
                user->write(S2C_ANS_NEWPLAYER, "b", NA_ERROR);
                return;
            }

            if (job >= CLASS_NUM)
            {
                user->write(S2C_ANS_NEWPLAYER, "b", NA_WRONGCLASS);
                return;
            }

            if (str + hth + int_ + wis + dex != 5)
            {
                user->write(S2C_ANS_NEWPLAYER, "b", NA_WRONGPROPERTY);
                return;
            }

            if (hair > 6 || face > 6)
            {
                user->write(S2C_ANS_NEWPLAYER, "b", NA_WRONGPROPERTY);
                return;
            }

            // TODO: Merge both into one struct.
            copy.push<int>(user->GetAID());
            copy.push<unsigned int>(user->GetUID());

            m_dbclient.write(copy.change_type(S2D_NEWPLAYER));
        });

        m_gameserver.when(C2S_DELPLAYER, [&](const std::unique_ptr<Player>& user, packet& p) {
            if (user->InGame()) return;
            p.push<int>(user->GetAID());
            p.push<unsigned int>(user->GetUID());

            m_dbclient.write(p.change_type(S2D_DELPLAYER));
        });

        m_gameserver.when(C2S_RESTOREPLAYER, [&](const std::unique_ptr<Player>& user, packet& p) {
            if (user->InGame()) return;
            p.push<int>(user->GetAID());
            p.push<unsigned int>(user->GetUID());

            m_dbclient.write(p.change_type(S2D_RESTOREPLAYER));
        });

        m_gameserver.when(C2S_LOADPLAYER, [&](const std::unique_ptr<Player>& user, packet& p) {
            if (user->InGame()) return;
            p.push<int>(user->GetAID());
            p.push<unsigned int>(user->GetUID());

            m_dbclient.write(p.change_type(S2D_LOADPLAYER));
        });

        m_gameserver.when(C2S_START, [&](const std::unique_ptr<Player>& user, packet& p) {
            // TODO: Add check if already loaded.
            if (!user->InGame()) return;
            user->GameStart(p);
        });

        m_gameserver.when(C2S_RESTART, [&](const std::unique_ptr<Player>& user, packet& p) {
            if (!user->InGame()) return;
            if (p.pop<char>() == 1) // Can I logout?
                user->write(S2C_ANS_RESTART, "b", user->CanLogout() ? 1 : 0); // 1=Yes, 0=No -> In Fight? PVP? Etc
            else
            {
                user->GameRestart();
                user->DestroyPlayer();
                m_dbclient.write(S2D_RESTART, "dd", user->GetUID(), user->GetAID());
            }
        });

        m_gameserver.on_connected([&](const std::unique_ptr<Player>& user) {
            std::cout << "connection: " << user->GetUID() << std::endl;
        });

        m_gameserver.on_disconnected([&](const std::unique_ptr<Player>& user) {
            std::cout << "disconnection: " << user->GetUID() << std::endl;
        });
    }
};

int main()
{
    GameManager manager;

    manager.ConnectToDatabase   ("localhost", 2999);
    manager.StartGameServer     ("localhost", 3000);

    std::cin.get();
    return 0;
}