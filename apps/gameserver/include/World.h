#pragma once

#include <unordered_map>
#include <functional>

#include "Player.h"
#include "Monster.h"
#include "NPC.h"
#include "Spawn.h"

#include <bango/space/quadtree.h>

#define MAP_WIDTH 50*8192
#define MAP_SIGHT 1024

class WorldMap
{
public:
    class Container : public bango::space::quad_entity_container<Container>
    {
    public:
        using CharacterContainer = std::unordered_map<unsigned int, Character *>;
    private:
        std::unordered_map<char, CharacterContainer> m_entities;

    public:

        Container();

        const CharacterContainer& players()     const { return m_entities.at(Character::PLAYER);    }
        const CharacterContainer& monsters()    const { return m_entities.at(Character::MONSTER);   }
        const CharacterContainer& npcs()        const { return m_entities.at(Character::NPC);       }
        const CharacterContainer& loots()       const { return m_entities.at(Character::LOOT);      }

        void insert(const bango::space::quad_entity* entity) override;
        void remove(const bango::space::quad_entity* entity) override;
        void merge(const Container* container) override;
        size_t size() const override;
        long long total_memory() const override;
        void for_each(const std::function<void(const bango::space::quad_entity*)>&& callback) const override;
    };

private:
    const int m_sight;

public:
    using AppearEvent = std::function<void (Player &, Character &, bool)>;
    using DisappearEvent = std::function<void (Player &, Character &)>;
    using MoveEvent = std::function<void (Player &, Character &, std::int8_t, std::int8_t, std::int8_t, bool)>;

private:
    bango::space::quad<Container> m_quad;

    // BUG: Add checks.
    AppearEvent     m_on_appear;//    =[](const Player*, const Character*){};
    DisappearEvent  m_on_disappear;// =[](const Player*, const Character*){};
    MoveEvent       m_on_move;//      =[](const Player*, const Character*, std::int8_t, std::int8_t, std::int8_t, bool){};

    std::unordered_map<char, Container::CharacterContainer> m_entities;

    std::recursive_mutex m_rmtx;

public:
    WorldMap(const int width, const int sight, const size_t max_container_entity=QUADTREE_MAX_NODES)
        : m_sight(sight), m_quad(bango::space::square{{0,0}, width}, max_container_entity)
    {
        m_entities[Character::PLAYER]={};
        m_entities[Character::MONSTER]={};
        m_entities[Character::NPC]={};
        m_entities[Character::LOOT]={};
    }

    enum QUERY_KIND
    {
        QK_PLAYER   = 1 << Character::PLAYER,
        QK_MONSTER  = 1 << Character::MONSTER,
        QK_NPC      = 1 << Character::NPC,
        QK_LOOT     = 1 << Character::LOOT,
    };

    //! Non thread-safe.
    const Container::CharacterContainer& Players()  const { return m_entities.at(Character::PLAYER);    }
    const Container::CharacterContainer& Monsters() const { return m_entities.at(Character::MONSTER);   }
    const Container::CharacterContainer& Npcs()     const { return m_entities.at(Character::NPC);       }
    const Container::CharacterContainer& Loots()    const { return m_entities.at(Character::LOOT);      }

    //! Adds new entity to the map.
    //! Thread-safe.
    void Add(Character* entity);

    //! Removes entity from the map.
    //! Thread-safe.
    void Remove(Character* entity, bool on_monster_death = false);

    //! Moves entity to new position.
    //! Thread-safe.
    void Move(Character* entity, std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z=0, bool stop=false);

    void For(QUERY_KIND kind, Character::id_t id, const std::function<void(Character&)>&& callback);

    //! Executes callback for all players around in given radius.
    //! Thread-safe.
    void ForEachPlayerAround(const bango::space::quad_entity& qe, unsigned int radius, const std::function<void(Player&)>&& callback);

    //! Executes callback for all queried entities around in given radius.
    //! Thread-safe.
    void ForEachAround(const bango::space::quad_entity& qe, unsigned int radius, QUERY_KIND kind, const std::function<void(Character&)>&& callback);

    //! Writes packet to players in sight.
    //! Thread-safe.
    void WriteInSight(const bango::space::quad_entity& qe, const bango::network::packet& p);
    
    //! Writes packet to all players on this WorldMap.
    //! Thread-safe.
    void WriteOnMap(const bango::network::packet& p);

    //! Registers callbacks for appearance events.
    void OnAppear       (const AppearEvent&     callback){ m_on_appear       = callback; }
    void OnDisappear    (const DisappearEvent&  callback){ m_on_disappear    = callback; }
    void OnMove         (const MoveEvent&       callback){ m_on_move         = callback; }
};

class World
{
    constexpr static unsigned short MAP_COUNT = 32;

    std::vector<std::unique_ptr<WorldMap>> m_maps;

    using PlayerContainer = std::unordered_map<Character::id_t, Player *>;
    using MonsterContainer = std::unordered_map<Character::id_t, std::shared_ptr<Monster>>;
    using NpcContainer = std::unordered_map<Character::id_t, std::shared_ptr<NPC>>;
    using SpawnContainer = std::list<std::shared_ptr<Spawn>>;
    using PartyContainer = std::unordered_map<Character::id_t, std::shared_ptr<Party>>;

    PlayerContainer m_players;
    MonsterContainer m_monsters;
    NpcContainer m_npcs;
    SpawnContainer m_spawns;
    PartyContainer m_parties;

    std::recursive_mutex m_entities_rmtx;

    World()
    {
        for (int i = 0; i < MAP_COUNT; i++)
            m_maps.push_back(std::make_unique<WorldMap>(MAP_WIDTH, MAP_SIGHT));
    }

    static World& Get()
    {
        static World instance;
        return instance;
    }

public:

    static WorldMap& Map(size_t id)
    {
        return *Get().m_maps[id >= MAP_COUNT ? 0 : id].get();
    }

    static void OnAppear(const WorldMap::AppearEvent& callback)
    {
        for (auto& map : Get().m_maps)
            map->OnAppear(callback);
    }

    static void OnDisappear(const WorldMap::DisappearEvent& callback)
    {
        for (auto& map : Get().m_maps)
            map->OnDisappear(callback);
    }

    static void OnMove(const WorldMap::MoveEvent& callback)
    {
        for (auto& map : Get().m_maps)
            map->OnMove(callback);
    }

    static void SpawnNpcs()
    {
        for (const auto& init : InitNPC::DB())
            Add(std::make_shared<NPC>(init.second));
    }

    static void CreateSpawnsAndSpawnMonsters();

    static void Cleanup()
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);
        Get().m_players.clear();
        Get().m_spawns.clear();
        Get().m_monsters.clear();
        Get().m_npcs.clear();
    }

    static void RemoveDeadMonsters()
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

        for (auto it = Get().m_monsters.begin(); it != Get().m_monsters.end(); )
        {
            auto& monster = it->second;
            if (monster->IsGState(CGS_KO)) {
                Map(monster->GetMap()).Remove(monster.get(), true);
                it = Get().m_monsters.erase(it);
            }
            else
                ++it;
        }
    }

    //! Adds player to the world without ownership. 
    static void Add(Player* entity)
    {
        {
            std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);
            Get().m_players.insert(std::make_pair(entity->GetID(), entity));
        }
        Map(entity->GetMap()).Add(entity);
    }

    //! Adds characters to the world with ownership.
    static void Add(std::shared_ptr<Character> entity)
    {
        if (entity->GetType() == Character::PLAYER)
            throw std::logic_error("player should be added to the world by reference");

        {
            std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);
        
            switch(entity->GetType())
            {
            case Character::MONSTER:
                Get().m_monsters.insert(std::make_pair(entity->GetID(), std::dynamic_pointer_cast<Monster>(entity)));
            case Character::NPC:
                Get().m_npcs.insert(std::make_pair(entity->GetID(), std::dynamic_pointer_cast<NPC>(entity)));
            }

        }
        Map(entity->GetMap()).Add(entity.get());
    }

    //! Adds party to the world
    static void AddParty(std::shared_ptr<Party> party);
    
    //! Removes party from the world
    static void RemoveParty(std::shared_ptr<Party> party);

    //! Callback function for party
    static bool ForParty(Character::id_t id, const std::function<void(Party&)>& callback);


    //! Removes player from the world.
    static void Remove(Player* entity)
    {
        Map(entity->GetMap()).Remove(entity);
        {
            std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);
            Get().m_players.erase(entity->GetID());
        }
    }

    static void Move(Character* entity, std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z=0, bool stop=false)
    {
        Map(entity->GetMap()).Move(entity, delta_x, delta_y, delta_z, stop);
    }

    static void Teleport(Player* entity, int x, int y, int z, int spread=0, int map=-1)
    {
        // TODO: Add random spread.
        Map(entity->GetMap()).Remove(entity);
        entity->m_x = x;
        entity->m_y = y;
        entity->m_z = z;

        if (map >= 0 && map < MAP_COUNT)
            entity->m_map = map;

        Map(entity->GetMap()).Add(entity);
    }

    static void ForEachPlayer(const std::function<void(Player&)>& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

        for (auto it : Get().m_players)
            callback(*it.second);
    }

    static bool ForPlayerWithName(const std::string& name, const std::function<void(Player&)>& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

        auto it = std::find_if(Get().m_players.begin(), Get().m_players.end(), [&](auto& p){
            return p.second->GetName() == name;
        });

        if (it == Get().m_players.end())
            return false;

        callback(*(it->second));
        return true;
    }

    static bool ForPlayer(Character::id_t id, const std::function<void(Player&)>& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

        auto it = Get().m_players.find(id);
        if (it == Get().m_players.end())
            return false;
        
        callback(*it->second);
        return true;
    }

    static void ForEachMonster(const std::function<void(Monster&)>& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

        for (auto it : Get().m_monsters)
            callback(*it.second);
    }

    static bool ForMonster(Character::id_t id, const std::function<void(Monster&)>& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);  // TODO: Possibly have separate mutex for each container?

        auto it = Get().m_monsters.find(id);
        if (it == Get().m_monsters.end())
            return false;
        
        callback(*it->second);
        return true;
    }

    static void ForEachNpc(const std::function<void(NPC&)>& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

        for (auto it : Get().m_npcs)
            callback(*it.second);
    }

    static bool ForNpc(Character::id_t id, const std::function<void(NPC&)>& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

        auto it = Get().m_npcs.find(id);
        if (it == Get().m_npcs.end())
            return false;
        
        callback(*it->second);
        return true;
    }

    static void ForEachSpawn(const std::function<void(Spawn&)>& callback);
};

inline WorldMap::QUERY_KIND operator|(WorldMap::QUERY_KIND a, WorldMap::QUERY_KIND b)
{return static_cast<WorldMap::QUERY_KIND>(static_cast<int>(a) | static_cast<int>(b));}