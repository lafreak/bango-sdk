#include "Spawn.h"

#include "spdlog/spdlog.h"
#include "World.h"
#include "RegularMonster.h"
#include "BeheadableMonster.h"
#include "Character.h"

#include <inix/common.h>

using namespace bango::utils;

Spawn::Spawn(const std::unique_ptr<GenMonster>& init):
    m_init(init)
{
    if(InitMonster::DB().find(GetMonsterIndex()) != InitMonster::DB().end())
        CreateSpawn();
    else
        spdlog::error("Cannot create spawn of monster {}", GetMonsterIndex()); // TODO: object shouldn't be created.

    SetNextSpawnCycle();
}

void GenMonster::set(bango::processor::lisp::var param)
{
    switch (FindAttribute(param.pop()))
    {
        case A_INDEX:       MonsterIndex = param.pop(); break;
        case A_MAP:         Map          = param.pop(); break;
        case A_AREA:        Index        = param.pop(); break;
        case A_MAX:         Amount       = param.pop(); break;
        case A_SPAWNCYCLE:  SpawnCycle   = param.pop();
                            SpawnCycle   *= 1000;
                            break;
        case A_RECT:        Rect.X1      = param.pop();
                            Rect.Y1      = param.pop();
                            Rect.X2      = param.pop();
                            Rect.Y2      = param.pop();
                            break;
    }
}

void Spawn::Tick()
{
    if((time::now() - m_next_spawn_cycle).count() > GetSpawnCycle())
        return;

    for(auto& monster : m_area_monsters)
    {
        auto monster_lock = monster->Lock();
        if(monster->IsGState(CGS_KO))
            RespawnOnWorld(monster);
    }

    SetNextSpawnCycle();
}

void Spawn::RespawnOnWorld(std::shared_ptr<Monster> monster)
{
    monster->PrepareMonsterToSpawn(m_init->Rect.GetRandomX(), m_init->Rect.GetRandomY(), m_init->Map);
    World::Add(monster);
}

void Spawn::CreateSpawn()
{
    const auto& init_monster = InitMonster::DB().at(GetMonsterIndex());
    m_area_monsters.resize(GetAmount());
    for (int i = 0; i < GetAmount(); i++)
    {
        switch (init_monster->Race)
        {
            case MR_NOTMAGUNI:
            {
                auto monster = std::make_shared<RegularMonster>(init_monster, GetRandomX(), GetRandomY(), GetMap());
                m_area_monsters.at(i) = monster;
                World::Add(monster);
                break;
            }
            case MR_MAGUNI:
            { 
                auto monster = std::make_shared<BeheadableMonster>(init_monster, GetRandomX(), GetRandomY(), GetMap());
                m_area_monsters.at(i) = monster;
                World::Add(monster);
                break;
            }
            default:
            {
                auto monster = std::make_shared<RegularMonster>(init_monster, GetRandomX(), GetRandomY(), GetMap());
                m_area_monsters.at(i) = monster;
                World::Add(monster);
                break;
            }
        }
    }
}

void Spawn::SetNextSpawnCycle()
{
    m_next_spawn_cycle = (time::now() + time::duration(GetSpawnCycle()));
}

std::uint32_t Spawn::GetIndex() const
{
    return m_init->Index;
}

std::uint32_t Spawn::GetMonsterIndex() const
{
    return m_init->MonsterIndex;
}

std::uint32_t Spawn::GetMap() const
{
    return m_init->Map;
}

std::uint32_t Spawn::GetAmount() const
{
    return m_init->Amount;
}

std::uint32_t Spawn::GetSpawnCycle() const
{
    return m_init->SpawnCycle;
}

std::uint32_t Spawn::GetRandomX() const
{
    return m_init->Rect.GetRandomX();
}

std::uint32_t Spawn::GetRandomY() const
{
    return m_init->Rect.GetRandomY();
}

GenMonster::RectXY Spawn::GetRect() const
{
    return m_init->Rect;
}

std::uint32_t GenMonster::RectXY::GetRandomX() const
{
    return bango::utils::random::between(X1, X2) * 32;
}

std::uint32_t GenMonster::RectXY::GetRandomY() const
{
    return bango::utils::random::between(Y1, Y2) * 32;
}

unsigned int GenMonster::index() const
{
    return Index;
}