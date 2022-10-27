#include "Monster.h"

#include <iostream>
#include <cstdint>
#include <memory>

#include "spdlog/spdlog.h"

#include "World.h"
#include "RegularMonster.h"
#include "BeheadableMonster.h"

#include <bango/network/packet.h>

using namespace bango::network;

Monster::Monster(const std::unique_ptr<InitMonster>& init, int x, int y, int map)
    : Character(Character::MONSTER), m_init(init)
{
    spdlog::trace("Monster constructor id: {}", GetID());
    m_x = x;
    m_y = y;
    m_map = map;
    m_curhp = GetMaxHP();
    m_curmp = GetMaxMP();
}

Monster::~Monster()
{
    spdlog::trace("Monster destructor id: {}", GetID());
}

packet Monster::BuildAppearPacket(bool hero) const
{
    //                                wdddwddIIsbdsIIb
    return packet(S2C_CREATEMONSTER, "wdddwddIIsbdsIIb", 
        GetIndex(),
        GetID(),
        GetX(),
        GetY(),
        GetDir(),
        GetCurHP(),//200,//GetCurHP()
        GetMaxHP(),//200,//GetMaxHP()
        GetGState(),
        GetMState(),
        "\0",
        GetRace(),//TODO: | (hero ? GAME_HERO : 0),
        0, //gid
        "\0",
        GetGStateEx(),
        GetMStateEx(),
        GetLevel()
        );
}

packet Monster::BuildDisappearPacket() const
{
    return packet(S2C_REMOVEMONSTER, "d", GetID());
}

packet Monster::BuildMovePacket(std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop) const
{
    return packet();
}

void Monster::SummonMonster(std::uint32_t index, std::int32_t x, std::int32_t y, std::int32_t map)
{
    const auto& init_monster = InitMonster::DB().at(index);
    switch(init_monster->Race)
    {
        case MR_NOTMAGUNI:
        {
            World::Add(std::make_shared<RegularMonster>(init_monster, x, y, map));
            break;
        }
        case MR_MAGUNI:
        { 
            World::Add(std::make_shared<BeheadableMonster>(init_monster, x, y, map));
            break;
        }
        default:
        {
            World::Add(std::make_shared<RegularMonster>(init_monster, x, y, map));
            break;
        }
    }
}

void Monster::SetNewID()
{
    m_id = Character::g_max_id++;
}

void Monster::PrepareMonsterToSpawn(int new_x, int new_y, int map)
{
    ResetStates();
    SetNewID();
    m_curhp = GetMaxHP();
    m_x = new_x;
    m_y = new_y;
}

bango::utils::time::point Monster::GetDeathTime() const
{
    return m_death_time;
}
void Monster::SetDeathTime(bango::utils::time::point death_time)
{
    m_death_time = death_time;
}