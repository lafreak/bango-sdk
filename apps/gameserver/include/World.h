#pragma once

#include <bango/space/quadtree.h>

#include "Player.h"
#include "NPC.h"

#define MAP_WIDTH 50*8192
#define MAP_SIGHT 1024

class WorldMap
{
public:
    class Container : public bango::space::quad_entity_container<Container>
    {
        std::list<Player*>    m_players;
        std::list<NPC*>       m_npcs;

    public:
        const std::list<Player*>& players()   const { return m_players;   }
        const std::list<NPC*>&    npcs()      const { return m_npcs;      }

        // TODO: Add check if entity already exists. std::list->std::map?
        void insert(const bango::space::quad_entity* entity) override
        {
            // TODO: Change?
            switch (((Character*)entity)->GetType())
            {
                case Character::PLAYER: 
                    m_players.push_back((Player*) entity);
                    break;
                case Character::NPC:
                    m_npcs.push_back((NPC*) entity);
                    break;
            }
        }

        // TODO: Add check if entity already exists. std::list->std::map?
        void remove(const bango::space::quad_entity* entity) override
        {
            switch (((Character*)entity)->GetType())
            {
                case Character::PLAYER: 
                    m_players.remove((Player*) entity);
                    break;
                case Character::NPC: 
                    m_npcs.remove((NPC*) entity);
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
                sizeof(m_players)+m_players.size()*sizeof(Player*) +
                sizeof(m_npcs)+m_npcs.size()*sizeof(NPC*);
        }
        
        void for_each(const std::function<void(const bango::space::quad_entity*)>&& callback) const override
        {
            for (auto& qe : m_players)
                callback(qe);

            for (auto& qe : m_npcs)
                callback(qe);
        }
    };

private:
    const int m_sight;

public:
    typedef std::function<void(Player*, Character*)>    AppearanceEvent;
    typedef std::function<void(Player*, Character*, 
        std::int8_t, std::int8_t, std::int8_t, bool)>   MoveEvent;

private:
    bango::space::quad<Container> m_quad;

    // BUG: Add checks.
    AppearanceEvent   m_on_appear;//    =[](const Player*, const Character*){};
    AppearanceEvent   m_on_disappear;// =[](const Player*, const Character*){};
    MoveEvent         m_on_move;//      =[](const Player*, const Character*, std::int8_t, std::int8_t, std::int8_t, bool){};

    typedef std::map<unsigned int, Player*>   PlayerMap;
    typedef std::map<unsigned int, NPC*>      NPCMap;

    PlayerMap   m_players;
    NPCMap      m_npcs;

public:
    WorldMap(const int width, const int sight, const size_t max_container_entity=QUADTREE_MAX_NODES)
        : m_sight(sight), m_quad(bango::space::square{{0,0}, width}, max_container_entity)
    {
    }

    const PlayerMap&    Players()   const { return m_players; }
    const NPCMap&       NPCs()      const { return m_npcs; }

    void Add(Character* entity)
    {
        auto center = bango::space::point{entity->m_x, entity->m_y};
        m_quad.query(center, m_sight, [&](const Container* container) {
            for (auto& player : container->players())
            {
                if (player->distance(center) <= m_sight)
                {
                    m_on_appear(player, entity);
                    if (entity->GetType() == Character::PLAYER)
                        m_on_appear((Player*) entity, player);
                }
            }

            if (entity->GetType() != Character::PLAYER)
                return;

            for (auto& npc : container->npcs())
            {
                if (npc->distance(center) <= m_sight)
                    m_on_appear((Player*) entity, npc);
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
                m_players.insert(std::make_pair(entity->GetID(), (Player*)entity)); 
                break;
            case Character::NPC: 
                m_npcs.insert(std::make_pair(entity->GetID(), (NPC*)entity)); 
                break;
        }
    }

    void Remove(Character* entity)
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
            case Character::NPC: 
                m_npcs.erase(entity->GetID()); 
                break;
        }

        auto center = bango::space::point{entity->m_x, entity->m_y}; // TODO: Dont convert to point each time.
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

        auto old_center = bango::space::point{entity->m_x, entity->m_y};
        auto new_center = bango::space::point{entity->m_x+delta_x, entity->m_y+delta_y};

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
                        m_on_disappear((Player*) entity, player);
                }
            }

            if (entity->GetType() != Character::PLAYER)
                return;

            for (auto& npc : container->npcs())
            {
                if (npc->distance(old_center) <= m_sight && npc->distance(new_center) > m_sight) 
                    m_on_disappear((Player*) entity, npc);
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
                        m_on_appear((Player*) entity, player);
                }
            }

            if (entity->GetType() != Character::PLAYER)
                return;

            for (auto& npc : container->npcs())
            {
                if (npc->distance(old_center) > m_sight && npc->distance(new_center) <= m_sight) 
                    m_on_appear((Player*) entity, npc);
            }
        });

        try {
            m_quad.insert(entity);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void ForEachPlayerAround(const bango::space::quad_entity* qe, unsigned int radius, const std::function<void(Player*)>&& callback)
    {
        auto center = bango::space::point{qe->m_x, qe->m_y};
        m_quad.query(center, radius, [&](const Container* container) {
            for (auto& player : container->players()) {
                if (player->distance(center) <= radius)
                    callback(player);
            }
        });
    }

    void WriteInSight(const bango::space::quad_entity* qe, const bango::network::packet& p)
    {
        ForEachPlayerAround(qe, m_sight, [&](Player* player) {
            player->write(p);
        });
    }

    void WriteOnMap(const bango::network::packet& p)
    {
        for (const auto& pair : Players())
            pair.second->write(p);
    }

    void OnAppear       (const AppearanceEvent&    callback){ m_on_appear       = callback; }
    void OnDisappear    (const AppearanceEvent&    callback){ m_on_disappear    = callback; }
    void OnMove         (const MoveEvent&          callback){ m_on_move         = callback; }
};

class World
{
    constexpr static unsigned short MAP_COUNT = 32;

    // BUG: Stack vs heap
    std::vector<std::unique_ptr<WorldMap>> m_maps;

    std::map<char, std::map<unsigned int, Character*>> m_entities;

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
    static void OnAppear(const WorldMap::AppearanceEvent& callback)
    {
        for (auto& map : Get().m_maps)
            map->OnAppear(callback);
    }

    static void OnDisappear(const WorldMap::AppearanceEvent& callback)
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
        for (const auto& p : Npcs())
            delete p.second;
    }

    static void Add(Character* entity)
    {
        Get().m_entities[entity->GetType()].insert(std::make_pair(entity->GetID(), entity));
        Map(entity->GetMap()).Add(entity);
    }

    static void Remove(Character* entity)
    {
        Get().m_entities[entity->GetType()].erase(entity->GetID());
        Map(entity->GetMap()).Remove(entity);
    }

    static void Move(Character* entity, std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z=0, bool stop=false)
    {
        Map(entity->GetMap()).Move(entity, delta_x, delta_y, delta_z, stop);
    }

    static const std::map<unsigned int, Character*>& Players()  { return Get().m_entities[Character::PLAYER]; }
    static const std::map<unsigned int, Character*>& Npcs()     { return Get().m_entities[Character::NPC]; }
};