#include "World.h"

using namespace bango::space;
using namespace bango::network;

WorldMap::Container::Container()
{
    m_entities[Character::PLAYER]={};
    m_entities[Character::MONSTER]={};
    m_entities[Character::NPC]={};
    m_entities[Character::LOOT]={};
}

void WorldMap::Container::insert(const quad_entity* entity)
{
    auto character = (Character*) entity;
    m_entities[character->GetType()].insert(std::make_pair(character->GetID(), character));
}

void WorldMap::Container::remove(const quad_entity* entity)
{
    auto character = (Character*) entity;
    m_entities[character->GetType()].erase(character->GetID());
}

void WorldMap::Container::merge(const WorldMap::Container* container)
{
    for (const auto& p : container->m_entities)
        m_entities[p.first].insert(p.second.cbegin(), p.second.cend());
}

size_t WorldMap::Container::size() const
{
    size_t count=0;
    for (const auto& p : m_entities)
        count += p.second.size();
    return count;
}

long long WorldMap::Container::total_memory() const
{
    long long count=0;
    for (const auto& p : m_entities)
        count += sizeof(p.second)+p.second.size()*sizeof(Character*);
}

void WorldMap::Container::for_each(const std::function<void(const quad_entity*)>&& callback) const
{
    for (const auto& p : m_entities)
        for (auto& q : p.second)
            callback(q.second);
}

void WorldMap::Add(Character* entity)
{
    std::lock_guard<std::recursive_mutex> lock(m_rmtx);

    auto center = point{entity->m_x, entity->m_y};
    m_quad.query(center, m_sight, [&](const Container* container) {
        for (auto& p : container->players())
        {
            auto player = (Player*) p.second;

            if (player->distance(center) <= m_sight)
            {
                m_on_appear(player, entity, entity->GetType() == Character::MONSTER);
                if (entity->GetType() == Character::PLAYER)
                    m_on_appear((Player*) entity, player, false);
            }
        }
        if (entity->GetType() != Character::PLAYER)
            return;

        for (auto& p : container->npcs())
        {
            auto npc = p.second;

            if (npc->distance(center) <= m_sight)
                m_on_appear((Player*) entity, npc, false);
        }

        for (auto& p : container->monsters())
        {
            auto monster = p.second;

            if (monster->distance(center) <= m_sight)
                m_on_appear((Player*) entity, monster, false);
        }
    });

    try {
        m_quad.insert(entity);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
    }

    m_entities[entity->GetType()].insert(std::make_pair(entity->GetID(), entity));
}

void WorldMap::Remove(Character* entity)
{
    std::lock_guard<std::recursive_mutex> lock(m_rmtx);

    try {
        m_quad.remove(entity);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
    }

    m_entities[entity->GetType()].erase(entity->GetID());

    auto center = point{entity->m_x, entity->m_y}; // TODO: Dont convert to point each time.
    m_quad.query(center, m_sight, [&](const Container* container) {
        for (auto& p : container->players())
        {
            auto player = (Player*) p.second;

            if (player->distance(center) <= m_sight)
                m_on_disappear(player, entity);
        }
    });
}

void WorldMap::Move(Character* entity, std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop)
{
    std::lock_guard<std::recursive_mutex> lock(m_rmtx);

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
        for (auto& p : container->players())
        {
            auto player = (Player*) p.second;

            if (player->distance(old_center) <= m_sight && player->distance(new_center) > m_sight) 
            {
                m_on_disappear(player, entity);
                if (entity->GetType() == Character::PLAYER)
                    m_on_disappear((Player*) entity, player);
            }
        }

        if (entity->GetType() != Character::PLAYER)
            return;

        for (auto& p : container->npcs())
        {
            auto npc = p.second;

            if (npc->distance(old_center) <= m_sight && npc->distance(new_center) > m_sight) 
                m_on_disappear((Player*) entity, npc);
        }

        for (auto& p : container->monsters())
        {
            auto monster = p.second;

            if (monster->distance(old_center) <= m_sight && monster->distance(new_center) > m_sight) 
                m_on_disappear((Player*) entity, monster);
        }
    });

    m_quad.query(new_center, m_sight, [&](const Container* container) {
        for (auto& p : container->players())
        {
            auto player = (Player*) p.second;

            if (player->distance(old_center) <= m_sight && player->distance(new_center) <= m_sight) 
                m_on_move(player, entity, delta_x, delta_y, delta_z, stop);
            else if (player->distance(old_center) > m_sight && player->distance(new_center) <= m_sight) 
            {
                m_on_appear(player, entity, false);
                if (entity->GetType() == Character::PLAYER)
                    m_on_appear((Player*) entity, player, false);
            }
        }

        if (entity->GetType() != Character::PLAYER)
            return;

        for (auto& p : container->npcs())
        {
            auto npc = p.second;

            if (npc->distance(old_center) > m_sight && npc->distance(new_center) <= m_sight) 
                m_on_appear((Player*) entity, npc, false);
        }

        for (auto& p : container->monsters())
        {
            auto monster = p.second;

            if (monster->distance(old_center) > m_sight && monster->distance(new_center) <= m_sight) 
                m_on_appear((Player*) entity, monster, false);
        }
    });

    try {
        m_quad.insert(entity);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void WorldMap::For(QUERY_KIND kind, Character::id_t id, const std::function<void(Character*)>&& callback)
{
    std::lock_guard<std::recursive_mutex> lock(m_rmtx);

    if (kind & QK_PLAYER)
    {
        try {
            return callback(m_entities[Character::PLAYER].at(id));
        } catch (const std::exception&) {}
    }

    if (kind & QK_NPC)
    {
        try {
            return callback(m_entities[Character::NPC].at(id));
        } catch (const std::exception&) {}
    }

    if (kind & QK_LOOT)
    {
        try {
            return callback(m_entities[Character::LOOT].at(id));
        } catch (const std::exception&) {}
    }

    if (kind & QK_MONSTER)
    {
        try {
            return callback(m_entities[Character::MONSTER].at(id));
        } catch (const std::exception&) {}
    }
}

void WorldMap::ForEachPlayerAround(const quad_entity* qe, unsigned int radius, const std::function<void(Player*)>&& callback)
{
    std::lock_guard<std::recursive_mutex> lock(m_rmtx);

    auto center = point{qe->m_x, qe->m_y};
    m_quad.query(center, radius, [&](const Container* container) {
        for (auto& p : container->players()) {
            auto player = (Player*) p.second;

            if (player->distance(center) <= radius)
                callback(player);
        }
    });
}

void WorldMap::WriteInSight(const quad_entity* qe, const packet& p)
{
    ForEachPlayerAround(qe, m_sight, [&](Player* player) {
        player->write(p);
    });
}

void WorldMap::WriteOnMap(const packet& p)
{
    std::lock_guard<std::recursive_mutex> lock(m_rmtx);

    for (const auto& pair : Players())
        ((Player*)pair.second)->write(p);
}