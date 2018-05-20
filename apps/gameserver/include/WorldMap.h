#pragma once

#include <bango/space/quadtree.h>

#include <memory>

class Player : public bango::space::quad_entity {};
class Container : public bango::space::quad_entity_container<Container>
{
    std::list<const Player*> m_players;

public:
    const std::list<const Player*>& players() { return m_players; }

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
    std::unique_ptr<bango::space::quad<Container>> m_quad;

public:
    WorldMap(const int width, const size_t max_container_entity=QUADTREE_MAX_NODES)
    {
        m_quad = std::make_unique<bango::space::quad<Container>>(bango::space::square{{0,0}, width}, max_container_entity);
    }

    void Add(const bango::space::quad_entity* entity)
    {
        m_quad->insert(entity);
    }

    void Remove(const bango::space::quad_entity* entity)
    {
        m_quad->remove(entity);
    }

    void Move(bango::space::quad_entity* entity, int new_x, int new_y)
    {
        m_quad->remove(entity);
        entity->m_x = new_x;
        entity->m_y = new_y;
        m_quad->insert(entity);
    }
};