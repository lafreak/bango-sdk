#pragma once

#include <bango/space/quadtree.h>

#include "Player.h"
#include "Monster.h"
#include "NPC.h"

#define MAP_WIDTH 50*8192
#define MAP_SIGHT 1024

class WorldMap
{
public:
    class Container : public bango::space::quad_entity_container<Container>
    {
    public:
        typedef std::map<unsigned int, Character*> CharacterContainer;
    private:
        std::map<char, CharacterContainer> m_entities;

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
    typedef std::function<void(Player*, Character*, bool)>    AppearEvent;
    typedef std::function<void(Player*, Character*)>    DisappearEvent;
    typedef std::function<void(Player*, Character*, 
        std::int8_t, std::int8_t, std::int8_t, bool)>   MoveEvent;

private:
    bango::space::quad<Container> m_quad;

    // BUG: Add checks.
    AppearEvent     m_on_appear;//    =[](const Player*, const Character*){};
    DisappearEvent  m_on_disappear;// =[](const Player*, const Character*){};
    MoveEvent       m_on_move;//      =[](const Player*, const Character*, std::int8_t, std::int8_t, std::int8_t, bool){};

    std::map<char, Container::CharacterContainer> m_entities;

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
    void Remove(Character* entity);

    //! Moves entity to new position.
    //! Thread-safe.
    void Move(Character* entity, std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z=0, bool stop=false);

    //! Executes callback for all players around in given radius.
    //! Thread-safe.
    void ForEachPlayerAround(const bango::space::quad_entity* qe, unsigned int radius, const std::function<void(Player*)>&& callback);

    //! Writes packet to players in sight.
    //! Thread-safe.
    void WriteInSight(const bango::space::quad_entity* qe, const bango::network::packet& p);
    
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

    std::map<char, WorldMap::Container::CharacterContainer> m_entities;
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

    // TODO: Change
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
            Add(new NPC(init.second.get()));
    }

    static void Cleanup()
    {
        ForEachNpc([](NPC* npc) { delete npc; });
    }

    static void Add(Character* entity)
    {
        {
            std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);
            Get().m_entities[entity->GetType()].insert(std::make_pair(entity->GetID(), entity));
        }
        Map(entity->GetMap()).Add(entity);
    }

    static void Remove(Character* entity)
    {
        {
            std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);
            Get().m_entities[entity->GetType()].erase(entity->GetID());
        }
        Map(entity->GetMap()).Remove(entity);
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

    static const WorldMap::Container::CharacterContainer& Players()  { return Get().m_entities[Character::PLAYER];   }//Unsafe
    static void ForEachPlayer(const std::function<void(Player*)>& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

        for (auto& p : Get().m_entities[Character::PLAYER])
            callback((Player*) p.second);
    }
    static void ForPlayer(std::uint32_t id, const std::function<void(Player*)>& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

        try {
            callback( (Player*) Get().m_entities[Character::PLAYER].at(id));
        } catch (const std::exception&) {}
    }

    static const WorldMap::Container::CharacterContainer& Monsters() { return Get().m_entities[Character::MONSTER];  }//Unsafe
    static void ForEachMonster(const std::function<void(Monster*)>& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

        for (auto& p : Get().m_entities[Character::MONSTER])
            callback((Monster*) p.second);
    }
    static void ForMonster(std::uint32_t id, const std::function<void(Monster*)>& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

        try {
            callback( (Monster*) Get().m_entities[Character::MONSTER].at(id));
        } catch (const std::exception&) {}
    }

    static const WorldMap::Container::CharacterContainer& Npcs()     { return Get().m_entities[Character::NPC];      }//Unsafe
    static void ForEachNpc(const std::function<void(NPC*)>& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

        for (auto& p : Get().m_entities[Character::NPC])
            callback((NPC*) p.second);
    }
    static void ForNpc(std::uint32_t id, const std::function<void(NPC*)>& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

        try {
            callback( (NPC*) Get().m_entities[Character::NPC].at(id));
        } catch (const std::exception&) {}
    }

    static const WorldMap::Container::CharacterContainer& Loots()    { return Get().m_entities[Character::LOOT];     }//Unsafe
    // static void ForEachLoot(const std::function<void(Loot*)>& callback)
    // {
    //     std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

    //     for (auto& p : Get().m_entities[Character::LOOT])
    //         callback((Loot*) p.second);
    // }
    // static void ForLoot(std::uint32_t id, const std::function<void(Loot*)>& callback)
    // {
    //     std::lock_guard<std::recursive_mutex> lock(Get().m_entities_rmtx);

    //     try {
    //         callback( (Loot*) Get().m_entities[Character::LOOT].at(id));
    //     } catch (const std::exception&) {}
    // }
};