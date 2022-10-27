#include "Spawn.h"

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
    {
        m_area_monsters.resize(GetAmount());
        for(int i = 0; i < GetAmount(); i++)
        {
            auto monster = CreateMonster();
            m_area_monsters.at(i) = monster;
        }
    }
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
    for(auto& monster : m_area_monsters)
        if(monster->IsGState(CGS_KO) && IsMonsterReadyToRespawn(monster))
            RespawnOnWorld(monster);
}


void Spawn::RespawnOnWorld(std::shared_ptr<Monster> monster)
{
    monster->PrepareMonsterToSpawn(m_init->Rect.GetRandomX(), m_init->Rect.GetRandomY(), m_init->Map);
    World::Add(monster);
}

bool Spawn::IsMonsterReadyToRespawn(std::shared_ptr<Monster> monster)
{
    return ((time::now() - monster->GetDeathTime()).count() >= GetSpawnCycle());
}

std::shared_ptr<Monster> Spawn::CreateMonster()
{
    const auto& init_monster = InitMonster::DB().at(GetMonsterIndex());
    switch(init_monster->Race)
    {
        case MR_NOTMAGUNI:
        {
            auto monster = std::make_shared<RegularMonster>(init_monster, GetRandomX(), GetRandomY(), GetMap());
            World::Add(monster);
            return monster;
        }
        case MR_MAGUNI:
        { 
            auto monster = std::make_shared<BeheadableMonster>(init_monster, GetRandomX(), GetRandomY(), GetMap());
            World::Add(monster);
            return monster;
        }
        default:
        {
            auto monster = std::make_shared<RegularMonster>(init_monster, GetRandomX(), GetRandomY(), GetMap());
            World::Add(monster);
            return monster;
        }
    }
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

std::vector<std::shared_ptr<Monster>> Spawn::GetMonsterVector() const
{
    return m_area_monsters;
}

const std::unique_ptr<GenMonster>& Spawn::GetInitMonster() const
{
    return m_init;
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