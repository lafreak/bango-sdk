#include <bango/network/client.h>
#include <bango/network/server.h>

#include <bango/space/quadtree.h>
#include <bango/processor/db.h>

#include <inix/protocol.h>
#include <inix/common.h>
#include <inix/structures.h>
#include <inix/attributes.h>

#include <iostream>
#include <cassert>
#include <map>
#include <sstream>

using namespace bango::network;
using namespace bango::space;
using namespace bango::processor;

#define MAP_WIDTH 50*8192
#define MAP_SIGHT 1024

struct InitItem : public db_object<InitItem>
{
    unsigned int 
        Index,
        Class,
        Kind,
        Level,
        Endurance;

    unsigned int index() const { return Index; }

    virtual void set(lisp::var param) override
    {
        switch (FindAttribute(param.pop()))
        {
            case A_INDEX:           Index         = param.pop(); break;
            case A_LEVEL:           Level         = param.pop(); break;
            case A_ENDURANCE:       Endurance     = param.pop(); break;
            case A_CLASS:           Class         = FindAttribute(param.pop());
                                    Kind          = FindAttribute(param.pop());
                                    break;
        }
    }
};

struct InitNPC : public db_object<InitNPC>
{
    unsigned int
        Index,
        Kind,
        Shape,
        HTML,
        Map,
        X, Y, Z,
        DirX, DirY;

    unsigned int index() const { return Index; }

    virtual void set(lisp::var param) override
    {
        switch (FindAttribute(param.pop()))
        {
            case A_INDEX:       Index       = param.pop(); break;
            case A_KIND:        Kind        = param.pop(); break;
            case A_SHAPE:       Shape       = param.pop(); break;
            case A_HTML:        HTML        = param.pop(); break;
            case A_MAP:         Map         = param.pop(); break;

            case A_XY:          X           = param.pop();
                                Y           = param.pop();
                                Z           = param.pop();
                                break;

            case A_DIR:         DirX        = param.pop();
                                DirY        = param.pop();
                                break;
        }
    }
};

class User : public writable, public authorizable
{
public:
    constexpr static int CAN_REQUEST_PRIMARY    = (1 << 0);
    constexpr static int CAN_REQUEST_SECONDARY  = (1 << 1);

    constexpr static int LOBBY      = (1 << 2);
    constexpr static int LOADING    = (1 << 3);
    constexpr static int INGAME     = (1 << 4);

    constexpr static int AUTHORIZED = (1 << 5);

    User(const taco_client_t& client) : writable(client)
    {
        // BUG: Not thread safe.
        static unsigned int g_max_uid=0;
        m_credentials.UID = g_max_uid++;
    }

    struct CREDENTIALS 
    {
        std::int32_t    AID; // AccountID   - Player DB Table Index
        std::uint32_t   UID; // UserID      - DB/Game Server Unique User Identifier
    } m_credentials;

    unsigned int        GetUID()        const { return m_credentials.UID; }
    int                 GetAID()        const { return m_credentials.AID; }
    const CREDENTIALS&  GetCredentials()const { return m_credentials; }

    void            SetAID(int value)   { m_credentials.AID = value; }
};

class Character : public quad_entity
{
    std::uint32_t   m_id;
    std::uint8_t    m_type;
    std::uint16_t   m_dir;

public:
    int             m_z;

public:
    Character(std::uint8_t type) : m_type(type)
    {
        //BUG: Not thread safe.
        static std::uint32_t g_max_id=0;
        m_id = g_max_id++;
    }

    constexpr static std::uint8_t PLAYER    =0;
    constexpr static std::uint8_t MONSTER   =1;
    constexpr static std::uint8_t NPC       =2;
    constexpr static std::uint8_t LOOT      =3;

    std::uint32_t   GetID()     const { return m_id; }
    std::uint8_t    GetType()   const { return m_type; }
    int             GetX()      const { return m_x; }
    int             GetY()      const { return m_y; }
    int             GetZ()      const { return m_z; }
    std::uint16_t   GetDir()    const { return m_dir; }

    virtual packet BuildAppearPacket(bool hero=false)   const { return packet(); };
    virtual packet BuildDisappearPacket()               const { return packet(); };
    virtual packet BuildMovePacket(std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop) const { return packet(); };

    void LookAt(int x, int y) { SetDirection(x - m_x, y - m_y); }
    void SetDirection(std::int8_t delta_x, std::int8_t delta_y)
    {
        if (delta_x == 0 && delta_y == 0) return;

        float absolute_x = abs(delta_x);
        float absolute_y = abs(delta_y);

        if (absolute_x >= absolute_y && absolute_x > 127) {
            delta_y = 127 * delta_y / absolute_x;
            delta_x = (((delta_x <= 0) - 1) & 0xFE) - 127;
        }
        else if (absolute_x < absolute_y && absolute_y > 127) {
            delta_x = 127 * delta_x / absolute_y;
            delta_y = (((delta_y <= 0) - 1) & 0xFE) - 127;
        }

        m_dir = delta_y + ((delta_x << 8) & 0xFF00);
    }
};

class NPC : public Character
{
    const InitNPC* m_init;
public:
    NPC(const InitNPC* init) 
        : Character(Character::NPC), m_init(init)
    {
        m_x = init->X;
        m_y = init->Y;
        m_z = init->Z;

        LookAt(init->DirX, init->DirY);
    }

    unsigned short  GetIndex() const { return m_init->Index; }
    unsigned char   GetShape() const { return m_init->Shape; }

    packet BuildAppearPacket(bool hero=false) const override
    {

        packet p(S2C_CREATENPC);
        p   << GetID()
            << GetIndex()
            << GetShape()
            << GetX()
            << GetY()
            << GetZ()
            << GetDir()
            << (std::int64_t) 0 // GState
            << (std::uint32_t) 0 // FlagItem
            << (std::uint32_t) 0 // Unknown
            ;
        return p;
    }

    packet BuildDisappearPacket() const override
    {
        packet p(S2C_REMOVENPC);
        p << GetID();
        return p;
    }
};

class Player : public Character, public User
{
    PLAYERINFO m_data;
    std::string m_name;
public:
    Player(const taco_client_t& client)
        : User(client), Character(Character::PLAYER) {}

    void OnLoadPlayer(packet& p)
    {
        p >> m_data >> m_name >> m_x >> m_y;
        m_z = m_data.Z;

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

    bool CanLogout() const { return true; }

    packet BuildAppearPacket(bool hero=false) const override
    {
        packet p(S2C_CREATEPLAYER);

        p   << GetID()
            << GetName()
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
    
    packet BuildDisappearPacket() const override
    {
        packet p(S2C_REMOVEPLAYER);
        p << GetID();
        return p;
    }

    packet BuildMovePacket(std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop) const override
    {
        packet p(stop ? S2C_MOVEPLAYER_END : S2C_MOVEPLAYER_ON);
        p << GetID() << delta_x << delta_y << delta_z;
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
    std::int32_t        GetZ()                      const { return m_z; }
    std::uint8_t        GetFace()                   const { return m_data.Face; }
    std::uint8_t        GetHair()                   const { return m_data.Hair; }

    // Not implemented
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

class WorldMap
{
public:
    class Container : public quad_entity_container<Container>
    {
        std::list<const Player*>    m_players;
        std::list<const NPC*>       m_npcs;

    public:
        const std::list<const Player*>& players()   const { return m_players;   }
        const std::list<const NPC*>&    npcs()      const { return m_npcs;      }

        // TODO: Add check if entity already exists. std::list->std::map?
        void insert(const quad_entity* entity) override
        {
            switch (((Character*)entity)->GetType())
            {
                case Character::PLAYER: 
                    m_players.push_back((const Player*) entity);
                    break;
                case Character::NPC:
                    m_npcs.push_back((const NPC*) entity);
                    break;
            }
        }

        // TODO: Add check if entity already exists. std::list->std::map?
        void remove(const quad_entity* entity) override
        {
            switch (((Character*)entity)->GetType())
            {
                case Character::PLAYER: 
                    m_players.remove((const Player*) entity);
                    break;
                case Character::NPC: 
                    m_npcs.remove((const NPC*) entity);
                    break;
            }
        }

        void merge(const Container* container) override
        {
            m_players   .insert(m_players.end(),    container->m_players.begin(),   container->m_players.end()  );
            m_npcs      .insert(m_npcs.end(),       container->m_npcs.begin(),      container->m_npcs.end()     );
        }

        size_t size() const override
        {
            return m_players.size() + m_npcs.size();
        }

        long long total_memory() const override
        {
            return 
                sizeof(m_players)+m_players.size()*sizeof(const Player*) +
                sizeof(m_npcs)+m_npcs.size()*sizeof(const NPC*);
        }
        
        void for_each(const std::function<void(const quad_entity*)>&& callback) const override
        {
            for (auto& qe : m_players)
                callback(qe);

            for (auto& qe : m_npcs)
                callback(qe);
        }
    };

private:
    const int m_sight;

    typedef std::function<void(const Player*, const Character*)>    AppearanceEvent;
    typedef std::function<void(const Player*, const Character*, 
        std::int8_t, std::int8_t, std::int8_t, bool)>               MoveEvent;

    quad<Container> m_quad;

    AppearanceEvent   m_on_appear    =[](const Player*, const Character*){};
    AppearanceEvent   m_on_disappear =[](const Player*, const Character*){};
    MoveEvent         m_on_move      =[](const Player*, const Character*, std::int8_t, std::int8_t, std::int8_t, bool){};

    typedef std::map<unsigned int, const Player*> PlayerMap;

    PlayerMap m_players;

public:
    WorldMap(const int width, const int sight, const size_t max_container_entity=QUADTREE_MAX_NODES)
        : m_sight(sight), m_quad(square{{0,0}, width}, max_container_entity)
    {
    }

    const PlayerMap& Players() const { return m_players; }

    void Add(const Character* entity)
    {
        auto center = point{entity->m_x, entity->m_y};
        m_quad.query(center, m_sight, [&](const Container* container) {
            for (auto& player : container->players())
            {
                if (player->distance(center) <= m_sight)
                {
                    m_on_appear(player, entity);
                    if (entity->GetType() == Character::PLAYER)
                        m_on_appear((const Player*) entity, player);
                }
            }

            if (entity->GetType() != Character::PLAYER)
                return;

            for (auto& npc : container->npcs())
            {
                if (npc->distance(center) <= m_sight)
                    m_on_appear((const Player*) entity, npc);
            }
        });
        
        try {
            m_quad.insert(entity);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return;
        }

        switch (entity->GetType())
        {
            case Character::PLAYER: 
                m_players.insert(std::make_pair(entity->GetID(), (const Player*)entity)); 
                break;
        }
    }

    void Remove(const Character* entity)
    {
        try {
            m_quad.remove(entity);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return;
        }

        switch (entity->GetType())
        {
            case Character::PLAYER: 
                m_players.erase(entity->GetID()); 
                break;
        }

        auto center = point{entity->m_x, entity->m_y}; // TODO: Dont convert to point each time.
        m_quad.query(center, m_sight, [&](const Container* container) {
            for (auto& player : container->players())
            {
                if (player->distance(center) <= m_sight)
                    m_on_disappear(player, entity);
            }
        });
    }

    void Move(Character* entity, std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z=0, bool stop=false)
    {
        try {
            m_quad.remove(entity);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return;
        }

        auto old_center = point{entity->m_x, entity->m_y};
        auto new_center = point{entity->m_x+delta_x, entity->m_y+delta_y};

        entity->m_x = new_center.x;
        entity->m_y = new_center.y;
        entity->m_z += delta_z;

        entity->SetDirection(delta_x, delta_y);

        // TODO: Add some margin.
        m_quad.query(old_center, m_sight, [&](const Container* container) {
            for (auto& player : container->players())
            {
                if (player->distance(old_center) <= m_sight && player->distance(new_center) > m_sight) 
                {
                    m_on_disappear(player, entity);
                    if (entity->GetType() == Character::PLAYER)
                        m_on_disappear((const Player*) entity, player);
                }
            }

            if (entity->GetType() != Character::PLAYER)
                return;

            for (auto& npc : container->npcs())
            {
                if (npc->distance(old_center) <= m_sight && npc->distance(new_center) > m_sight) 
                    m_on_disappear((const Player*) entity, npc);
            }
        });

        m_quad.query(new_center, m_sight, [&](const Container* container) {
            for (auto& player : container->players())
            {
                if (player->distance(old_center) <= m_sight && player->distance(new_center) <= m_sight) 
                    m_on_move(player, entity, delta_x, delta_y, delta_z, stop);
                else if (player->distance(old_center) > m_sight && player->distance(new_center) <= m_sight) 
                {
                    m_on_appear(player, entity);
                    if (entity->GetType() == Character::PLAYER)
                        m_on_appear((const Player*) entity, player);
                }
            }

            if (entity->GetType() != Character::PLAYER)
                return;

            for (auto& npc : container->npcs())
            {
                if (npc->distance(old_center) > m_sight && npc->distance(new_center) <= m_sight) 
                    m_on_appear((const Player*) entity, npc);
            }
        });

        try {
            m_quad.insert(entity);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void ForEachPlayerAround(const quad_entity* qe, unsigned int radius, const std::function<void(const Player*)>&& callback)
    {
        auto center = point{qe->m_x, qe->m_y};
        m_quad.query(center, radius, [&](const Container* container) {
            for (auto& player : container->players()) {
                if (player->distance(center) <= radius)
                    callback(player);
            }
        });
    }

    void WriteInSight(const quad_entity* qe, const packet& p)
    {
        ForEachPlayerAround(qe, m_sight, [&](const Player* player) {
            player->write(p);
        });
    }

    void WriteOnMap(const packet& p)
    {
        for (const auto& pair : Players())
            pair.second->write(p);
    }

    void OnAppear       (const AppearanceEvent&&    callback){ m_on_appear       = callback; }
    void OnDisappear    (const AppearanceEvent&&    callback){ m_on_disappear    = callback; }
    void OnMove         (const MoveEvent&&          callback){ m_on_move         = callback; }
};

class CommandDispatcher
{
public:
    class Token
    {
        constexpr static char DELIMETER = ' ';
        std::istringstream m_stream;

        std::string token() {
            std::string tok;
            m_stream >> std::ws;
            std::getline(m_stream, tok, DELIMETER);
            return tok;
        }

    public:
        Token(const std::string& str) : m_stream(str) {}

        operator std::string() 
        {
            return token();
        }

        operator int()
        {
            try {
                return std::stoi(token());
            } catch (const std::exception&) {
                return 0;
            }
        }

        operator float()
        {
            try {
                return std::stof(token());
            } catch (const std::exception&) {
                return 0.0;
            }
        }
    };

private:
    typedef const std::function<void(Player*, Token&)> Task;
    std::map<std::string, Task> m_commands;

public:
    void Register(const std::string& command, Task&& task)
    {
        m_commands.insert(std::make_pair(command, task));
    }

    void Dispatch(Player* player, const std::string& message)
    {
        Token token(message);
        std::string lookup = token;
        
        auto result = m_commands.find(lookup);
        if (result != m_commands.end())
            result->second(player, token);
    }
};

class GameManager
{
    client          m_dbclient;
    server<Player>  m_gameserver;

    WorldMap        m_map;

    CommandDispatcher m_command_dispatcher;

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

    GameManager() : m_map(MAP_WIDTH, MAP_SIGHT)
    {
        //!
        //! DBServer -> GameServer
        //!

        m_dbclient.when(D2S_LOGIN, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::unique_ptr<Player>& user) {
                user->assign(User::CAN_REQUEST_SECONDARY);
                user->write(p.change_type(S2C_ANS_LOGIN));
            });
        });

        // TODO: Bad naming, its not authorized
        m_dbclient.when(D2S_AUTHORIZED, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::unique_ptr<Player>& user) {
                user->assign(User::AUTHORIZED);
                user->SetAID(p.pop<int>());
            });
        });

        m_dbclient.when(D2S_SEC_LOGIN, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::unique_ptr<Player>& user) {
                user->assign(User::CAN_REQUEST_SECONDARY);
                user->write(p.change_type(S2C_SECOND_LOGIN));
            });
        });

        m_dbclient.when(D2S_PLAYER_INFO, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::unique_ptr<Player>& user) {
                user->write(p.change_type(S2C_PLAYERINFO));
                user->assign(User::LOBBY);
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
                user->OnLoadPlayer(p);
            });
        });

        m_dbclient.when(D2S_LOADITEMS, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::unique_ptr<Player>& user) {
                user->OnLoadItems(p);
            });
        });

        //!
        //! GameClient -> GameServer
        //!

        m_gameserver.when(C2S_CONNECT, [&](const std::unique_ptr<Player>& user, packet& p) {
            user->write(S2C_CODE, "dbdddIbbb", 0, 0, 604800, 0, 0, 0, 0, 0, N_EN);
        });

        m_gameserver.when(C2S_ANS_CODE, [&](const std::unique_ptr<Player>& user, packet& p) {
            // ignore...
        });

        m_gameserver.when(C2S_LOGIN, [&](const std::unique_ptr<Player>& user, packet& p) {
            user->deny(User::CAN_REQUEST_PRIMARY);
            p << user->GetUID();
            m_dbclient.write(p.change_type(S2D_LOGIN));
        });

        m_gameserver.when(C2S_SECOND_LOGIN, [&](const std::unique_ptr<Player>& user, packet& p) {
            user->deny(User::CAN_REQUEST_SECONDARY);
            p << user->GetCredentials();
            m_dbclient.write(p.change_type(S2D_SECONDARY_LOGIN));
        });

        m_gameserver.when(C2S_NEWPLAYER, [&](const std::unique_ptr<Player>& user, packet& p) {

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
            p << user->GetCredentials();

            m_dbclient.write(p.change_type(S2D_DELPLAYER));
        });

        m_gameserver.when(C2S_RESTOREPLAYER, [&](const std::unique_ptr<Player>& user, packet& p) {
            p << user->GetCredentials();

            m_dbclient.write(p.change_type(S2D_RESTOREPLAYER));
        });

        m_gameserver.when(C2S_LOADPLAYER, [&](const std::unique_ptr<Player>& user, packet& p) {
            user->assign(User::LOADING);
            user->deny(User::LOBBY);
            
            p << user->GetCredentials();
            m_dbclient.write(p.change_type(S2D_LOADPLAYER));
        });

        m_gameserver.when(C2S_START, [&](const std::unique_ptr<Player>& user, packet& p) {
            user->GameStart(p);
            m_map.Add(user.get());

            user->assign(User::INGAME);
            user->deny(User::LOADING);
        });

        m_gameserver.when(C2S_RESTART, [&](const std::unique_ptr<Player>& user, packet& p) {
            if (p.pop<char>() == 1) // Can I logout?
                user->write(S2C_ANS_RESTART, "b", user->CanLogout() ? 1 : 0); // 1=Yes, 0=No -> In Fight? PVP? Etc
            else
            {
                m_map.Remove(user.get());
                user->deny(User::INGAME);
                m_dbclient.write(S2D_RESTART, "dd", user->GetUID(), user->GetAID());
            }
        });

        m_gameserver.when(C2S_GAMEEXIT, [&](const std::unique_ptr<Player>& user, packet& p) {
            user->write(S2C_ANS_GAMEEXIT, "b", user->CanLogout() ? 1 : 0);
        });

        m_gameserver.when(C2S_MOVE_ON, [&](const std::unique_ptr<Player>& user, packet& p) {
            std::int8_t x, y, z;
            p >> x >> y >> z;
            m_map.Move(user.get(), x, y, z);
        });

        m_gameserver.when(C2S_MOVE_END, [&](const std::unique_ptr<Player>& user, packet& p) {
            std::int8_t x, y, z;
            p >> x >> y >> z;
            m_map.Move(user.get(), x, y, z, true);
        });

        m_gameserver.when(C2S_CHATTING, [&](const std::unique_ptr<Player>& user, packet& p) {
            auto message = p.pop_str(); // BUG: Empty packet will throw an exception.

            // if (message[0] == '/') // BUG: Message might be empty.
            if (message[0] == '/')
                return m_command_dispatcher.Dispatch(user.get(), message);

            packet out(S2C_CHATTING);
            out << user->GetName() << message;

            m_map.WriteInSight(user.get(), out);
        });

        m_gameserver.on_connected([&](const std::unique_ptr<Player>& user) {
            user->assign(User::CAN_REQUEST_PRIMARY);
            std::cout << "connection: " << user->GetUID() << std::endl;
        });

        m_gameserver.on_disconnected([&](const std::unique_ptr<Player>& user) {
            if (user->authorized(User::INGAME))
                m_map.Remove(user.get());

            if (user->authorized(User::AUTHORIZED))
                m_dbclient.write(S2D_DISCONNECT, "d", user->GetAID());

            std::cout << "disconnection: " << user->GetUID() << std::endl;
        });

        m_gameserver.grant({
            {C2S_LOGIN,             User::CAN_REQUEST_PRIMARY},
            {C2S_SECOND_LOGIN,      User::CAN_REQUEST_SECONDARY},

            {C2S_NEWPLAYER,         User::LOBBY},
            {C2S_DELPLAYER,         User::LOBBY},
            {C2S_RESTOREPLAYER,     User::LOBBY},
            {C2S_LOADPLAYER,        User::LOBBY},

            {C2S_START,             User::LOADING},

            {C2S_RESTART,           User::INGAME},
            {C2S_MOVE_ON,           User::INGAME},
            {C2S_MOVE_END,          User::INGAME},
            {C2S_CHATTING,          User::INGAME},
        });

        m_gameserver.restrict({
            
        });

        m_map.OnAppear([](const Player* receiver, const Character* subject) {
            // TODO: If it gets created for the first time (mob spawn) add true to param list.
            receiver->write(subject->BuildAppearPacket());
        });

        m_map.OnDisappear([](const Player* receiver, const Character* subject) {
            receiver->write(subject->BuildDisappearPacket());
        });

        m_map.OnMove([&](const Player* receiver, const Character* subject, 
            std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop) {
            receiver->write(subject->BuildMovePacket(delta_x, delta_y, delta_z, stop));
        });

        m_command_dispatcher.Register("/test", [&](Player* player, CommandDispatcher::Token& token) {
            //
        });

        InitItem    ::Load("Config/InitItem.txt");
        InitNPC     ::Load("Config/InitNPC.txt");

        for (auto& object : InitNPC::DB())
        {
            // Create NPC and place it on map.
            // TODO: Add to global map?
            // BUG: Memory leak
            m_map.Add(new NPC(object.second));
        }
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