#include "Monster.h"

#include <iostream>
#include <cstdint>
#include <memory>

#include "spdlog/spdlog.h"

#include "World.h"
#include "RegularMonster.h"
#include "BeheadableMonster.h"
#include "Party.h"

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

void Monster::Summon(std::uint32_t index, std::int32_t x, std::int32_t y, std::int32_t map)
{
    World::Add(Monster::CreateMonster(index, x, y, map));
}

std::shared_ptr<Monster> Monster::CreateMonster(std::uint32_t index, std::int32_t x, std::int32_t y, std::int32_t map)
{
    const auto& init_monster = InitMonster::DB().at(index);
    switch (init_monster->Race)
    {
        case MR_NOTMAGUNI:
            return std::make_shared<RegularMonster>(init_monster, x, y, map);
        case MR_MAGUNI:
            return std::make_shared<BeheadableMonster>(init_monster, x, y, map);
        default:
            return std::make_shared<RegularMonster>(init_monster, x, y, map);
    }
}

void Monster::RestoreInitialState(int new_x, int new_y)
{
    ResetStates();
    AssignNewId();
    m_curhp = GetMaxHP();
    m_x = new_x;
    m_y = new_y;
}

void Monster::ReceiveDamage(id_t id, std::uint32_t damage)
{
    Character::ReceiveDamage(id, damage);
    hostility_map[id] += damage;
    total_hostility += damage;
}


void Monster::DistributeExp()
{
    std::map<id_t, std::uint64_t> party_container;
    for (auto&[id, damage] : hostility_map)
    {
        World::ForPlayer(id, [&](Player& player){
            auto player_lock = player.Lock();
            if (player.GetPartyID() != 0)
                party_container[player.GetPartyID()] += damage;
            else
            {
                long double exp_share = static_cast<long double>(damage) / static_cast<long double>(total_hostility);
                std::uint64_t exp = exp_share * m_init->Exp;
                player.UpdateExp(player.CalculateExp(exp, GetLevel()));
            }
        });

    }

    hostility_map.clear();
    total_hostility = 0;
}

void Monster::Die()
{
    DistributeExp();
}