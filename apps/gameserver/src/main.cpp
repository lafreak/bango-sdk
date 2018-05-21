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
    bool m_ingame=false;

public:
    explicit User(const taco_client_t& client) : writable(client)
    {
        static unsigned int g_max_uid=0;
        m_credentials.UID = g_max_uid++;
    }

    struct CREDENTIALS 
    {
        std::int32_t    AID; // AccountID   - Player DB Table Index
        std::uint32_t   UID; // UserID      - DB/Game Server Unique User Identifier
    } m_credentials;

    bool InGame() const { return m_ingame; }

    void InitPlayer()           { m_ingame = true; } 
    void DestroyPlayer()        { m_ingame = false; }

    unsigned int        GetUID()        const { return m_credentials.UID; }
    int                 GetAID()        const { return m_credentials.AID; }
    const CREDENTIALS&  GetCredentials()const { return m_credentials; }

    void            SetAID(int value)   { m_credentials.AID = value; }
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
            GetCurHP(),
            GetCurHP(), //MaxHP
            GetCurMP(),
            GetCurMP(), //MaxMP
            1, //Hit
            2, //Dodge
            3, //Defense
            4, //Absorb
            GetExp(),
            5, //MinAttack
            6, //MaxAttack
            7, //MinMagic
            8, //MaxMagic
            GetPUPoint(),
            GetSUPoint(),
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

        write(BuildAppearPacket(true));
    }

    void GameRestart() {}

    bool CanLogout() const { return true; }

    packet BuildAppearPacket(bool hero=false)
    {
        packet p(S2C_CREATEPLAYER);

        p   << GetUID() // TODO: UID->CharacterID
            << m_name
            << GetClass(hero) 
            << GetX() 
            << GetY() 
            << GetZ() 
            << GetDir() 
            << GetGState();
        
        // Equipment Indexes
        p << (std::int16_t)0 << (std::int16_t)0 << (std::int16_t)0 << (std::int16_t)0;
        p << (std::int16_t)0 << (std::int16_t)0 << (std::int16_t)0 << (std::int16_t)0;

        p   << GetFace() 
            << GetHair() 
            << GetMState() 
            << "\0" << "\0" // GuildClass & GuildName
            << GetGID() 
            << GetFlag() 
            << GetFlagItem()
            << GetHonorGrade() 
            << GetHonorOption() 
            << GetGStateEx() 
            << GetMStateEx();

        // Unknown
        p << (std::int8_t)0 << (std::int32_t)0 << (std::int32_t)0 << (std::int8_t)0;

        return p;
    }

    const std::string&  GetName()                   const { return m_name; }
    std::uint8_t        GetClass(bool hero=false)   const { return hero ? (m_data.Class | GAME_HERO) : m_data.Class; }
    std::uint8_t        GetJob()                    const { return m_data.Job; }
    std::uint8_t        GetLevel()                  const { return m_data.Level; }
    std::uint16_t       GetStrength()               const { return m_data.Strength; }
    std::uint16_t       GetHealth()                 const { return m_data.Health; }
    std::uint16_t       GetInteligence()            const { return m_data.Inteligence; }
    std::uint16_t       GetWisdom()                 const { return m_data.Wisdom; }
    std::uint16_t       GetDexterity()              const { return m_data.Dexterity; }
    std::uint32_t       GetCurHP()                  const { return m_data.CurHP; }
    std::uint32_t       GetCurMP()                  const { return m_data.CurMP; }
    std::uint64_t       GetExp()                    const { return m_data.Exp; }
    std::uint16_t       GetPUPoint()                const { return m_data.PUPoint; }
    std::uint16_t       GetSUPoint()                const { return m_data.SUPoint; }
    std::uint16_t       GetContribute()             const { return m_data.Contribute; }
    std::uint32_t       GetRage()                   const { return m_data.Rage; }
    std::int32_t        GetX()                      const { return m_x; }
    std::int32_t        GetY()                      const { return m_y; }
    std::int32_t        GetZ()                      const { return m_data.Z; }
    std::uint8_t        GetFace()                   const { return m_data.Face; }
    std::uint8_t        GetHair()                   const { return m_data.Hair; }

    // Not implemented
    std::uint16_t       GetDir()        const { return 0; }
    std::uint64_t       GetGState()     const { return 0; }
    std::uint64_t       GetMState()     const { return 0; }
    std::uint32_t       GetGID()        const { return 0; }
    std::uint8_t        GetFlag()       const { return 0; }
    std::uint32_t       GetFlagItem()   const { return 0; }
    std::uint32_t       GetHonorGrade() const { return 0; }
    std::uint32_t       GetHonorOption()const { return 0; }
    std::uint64_t       GetGStateEx()   const { return 0; }
    std::uint64_t       GetMStateEx()   const { return 0; }
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

                // BUG: Player might log in by the time packet arrived?
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
            p << user->GetUID();
            m_dbclient.write(p.change_type(S2D_LOGIN));
        });

        m_gameserver.when(C2S_SECOND_LOGIN, [&](const std::unique_ptr<Player>& user, packet& p) {
            if (user->InGame()) return;
            p << user->GetCredentials();
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

            copy << user->GetCredentials();

            m_dbclient.write(copy.change_type(S2D_NEWPLAYER));
        });

        m_gameserver.when(C2S_DELPLAYER, [&](const std::unique_ptr<Player>& user, packet& p) {
            if (user->InGame()) return;
            p << user->GetCredentials();

            m_dbclient.write(p.change_type(S2D_DELPLAYER));
        });

        m_gameserver.when(C2S_RESTOREPLAYER, [&](const std::unique_ptr<Player>& user, packet& p) {
            if (user->InGame()) return;
            p << user->GetCredentials();

            m_dbclient.write(p.change_type(S2D_RESTOREPLAYER));
        });

        m_gameserver.when(C2S_LOADPLAYER, [&](const std::unique_ptr<Player>& user, packet& p) {
            if (user->InGame()) return;
            p << user->GetCredentials();

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
            m_dbclient.write(S2D_DISCONNECT, "d", user->GetAID());
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