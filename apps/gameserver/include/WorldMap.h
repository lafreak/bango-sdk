#pragma once

#include <bango/space/quadtree.h>

#include <memory>
#include <functional>

struct Player : public bango::space::quad_entity
{
    Player(int x, int y)
        : bango::space::quad_entity{x,y} {}
};

class Container : public bango::space::quad_entity_container<Container>
{
    std::list<const Player*> m_players;

public:
    const std::list<const Player*>& players() const { return m_players; }

    void insert(const bango::space::quad_entity* entity) override
    {
        m_players.push_back((const Player*) entity);
    }

    void remove(const bango::space::quad_entity* entity) override
    {
        m_players.remove((const Player*) entity);
    }

    void merge(const Container* container) override
    {
        m_players.insert(m_players.end(), container->m_players.begin(), container->m_players.end());
    }

    size_t size() const override
    {
        return m_players.size();
    }

    long long total_memory() const override
    {
        return sizeof(m_players)+m_players.size()*sizeof(const Player*);
    }
    
    void for_each(const std::function<void(const bango::space::quad_entity*)>&& callback) const override
    {
        for (auto& qe : m_players)
            callback(qe);
    }
};

class WorldMap
{
    const int m_sight;

    typedef std::function<void(const Player*, const bango::space::quad_entity*)>            AppearanceEvent;
    typedef std::function<void(const Player*, const bango::space::quad_entity*, int, int)>  MoveEvent;

    std::unique_ptr<bango::space::quad<Container>> m_quad;

    AppearanceEvent   m_on_appear    =[](const Player*, const bango::space::quad_entity*){};
    AppearanceEvent   m_on_disappear =[](const Player*, const bango::space::quad_entity*){};
    MoveEvent         m_on_move      =[](const Player*, const bango::space::quad_entity*, int, int){};

public:
    WorldMap(const int width, const int sight, const size_t max_container_entity=QUADTREE_MAX_NODES)
        : m_sight(sight)
    {
        m_quad = std::make_unique<bango::space::quad<Container>>(bango::space::square{{0,0}, width}, max_container_entity);
    }

    void Add(const bango::space::quad_entity* entity)
    {
        auto center = bango::space::point{entity->m_x, entity->m_y};
        m_quad->query(center, m_sight, [&](const Container* container) {
            for (auto& player : container->players())
            {
                if (player->distance(center) <= m_sight)
                {
                    m_on_appear(player, entity);
                    m_on_appear((const Player*) entity, player);
                }
            }
        });
        
        m_quad->insert(entity);
    }

    void Remove(const bango::space::quad_entity* entity)
    {
        m_quad->remove(entity);

        auto center = bango::space::point{entity->m_x, entity->m_y}; // TODO: Dont convert to point each time.
        m_quad->query(center, m_sight, [&](const Container* container) {
            for (auto& player : container->players())
            {
                if (player->distance(center) <= m_sight)
                {
                    m_on_disappear(player, entity);
                }
            }
        });
    }

    void Move(bango::space::quad_entity* entity, int new_x, int new_y)
    {
        m_quad->remove(entity);

        auto old_center = bango::space::point{entity->m_x, entity->m_y};
        auto new_center = bango::space::point{new_x, new_y};

        entity->m_x = new_x;
        entity->m_y = new_y;

        // TODO: Add some margin.
        m_quad->query(old_center, m_sight, [&](const Container* container) {
            for (auto& player : container->players())
            {
                if (player->distance(old_center) <= m_sight && player->distance(new_center) > m_sight) 
                {
                    m_on_disappear(player, entity);
                    m_on_disappear((const Player*) entity, player);
                }
            }
        });

        m_quad->query(new_center, m_sight, [&](const Container* container) {
            for (auto& player : container->players())
            {
                if (player->distance(old_center) <= m_sight && player->distance(new_center) <= m_sight) 
                    m_on_move(player, entity, new_x, new_y);
                else if (player->distance(old_center) > m_sight && player->distance(new_center) <= m_sight) 
                {
                    m_on_appear(player, entity);
                    m_on_appear((const Player*) entity, player);
                }
            }
        });

        m_quad->insert(entity);
    }

    void OnAppear       (const AppearanceEvent&& callback){ m_on_appear       = callback; }
    void OnDisappear    (const AppearanceEvent&& callback){ m_on_disappear    = callback; }
    void OnMove         (const MoveEvent&& callback)      { m_on_move         = callback; }
};