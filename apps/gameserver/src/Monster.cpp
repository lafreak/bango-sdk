#include "Monster.h"

#include <iostream>
#include <cstdint>
#include <memory>
#include <unordered_map>

#include "spdlog/spdlog.h"

#include "World.h"
#include "RegularMonster.h"
#include "BeheadableMonster.h"
#include "Party.h"

#include <bango/network/packet.h>
#include <bango/utils/random.h>

using namespace bango::network;
using namespace bango::processor;
using namespace bango::utils;


void InitMonster::set(lisp::var param)
{
    switch (FindAttribute(param.pop()))
    {
        case A_INDEX:       Index       = param.pop(); break;
        case A_RACE:        Race        = param.pop(); break;
        case A_LEVEL:       Level       = param.pop(); break;
        case A_AI:          AI          = param.pop(); break;
        case A_RANGE:       Range       = param.pop(); break;
        case A_SIGHT:       CloseSight  = param.pop();
                            FarSight    = param.pop();
                            break;
        case A_EXP:         Exp         = param.pop(); break;
        case A_STR:         Strength    = param.pop(); break;
        case A_HTH:         Health      = param.pop(); break;
        case A_INT:         Inteligence = param.pop(); break;
        case A_WIS:         Wisdom      = param.pop(); break;
        case A_DEX:         Dexterity   = param.pop(); break;
        case A_HP:          HP          = param.pop(); break;
        case A_MP:          MP          = param.pop(); break;
        case A_ASPEED:      AttackSpeed = param.pop(); break;
        case A_HIT:         Hit         = param.pop(); break;
        case A_DODGE:       Dodge       = param.pop(); break;
        case A_SIZE:        Size        = param.pop(); break;
        case A_ATTACK:      AttackType  = param.pop();
                            MinAttack   = param.pop();
                            MaxAttack   = param.pop();
                            break;
        case A_MAGICATTACK: MinMagic    = param.pop();
                            MaxMagic    = param.pop();
                            break;
        case IC_DEFENSE:    Defense[ATT_MEELE]  = param.pop();
                            Defense[ATT_RANGE]  = param.pop();
                            break;
        case A_ABSORB:      Absorb      = param.pop(); break;
        case A_SPEED:       WalkSpeed   = param.pop();
                            RunSpeed    = param.pop();
                            break;
                            //TODO: Not sure if order is right.
        case A_RESIST:      Resist[RT_FIRE]     = param.pop();
                            Resist[RT_ICE]      = param.pop();
                            Resist[RT_LITNING]  = param.pop();
                            Resist[RT_CURSE]    = param.pop();
                            Resist[RT_PALSY]    = param.pop();
                            break;
        case A_ITEMGROUP:
        {
            std::uint32_t itemgroup_index = param.pop();
            std::uint32_t number_of_rolls = param.pop();
            if(!ItemGroup::Find(itemgroup_index))
            {
                spdlog::warn("Cannot find itemgroup {} for monster {}", itemgroup_index, index());
                break;
            }

            m_itemgroups.push_back(MonsterItemGroup(itemgroup_index, number_of_rolls));
            break;
        }
    }
}

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


void Monster::AllotExpAndLoot()
{
    std::unordered_map<id_t, std::uint64_t> party_container;
    std::pair<id_t, std::uint64_t> max_player_damage = {0, 0};
    std::pair<id_t, std::uint64_t> max_party_damage = {0, 0};
    for (auto&[id, damage] : hostility_map)
    {
        World::ForPlayer(id, [&](Player& player){
            auto player_lock = player.Lock();
            if (player.GetPartyID() != 0)
                party_container[player.GetPartyID()] += damage;
            else
            {
                max_player_damage = max_player_damage.second < damage ? std::make_pair(id, damage) : max_player_damage;
                long double exp_share = static_cast<long double>(damage) / static_cast<long double>(total_hostility);
                std::uint64_t exp = exp_share * m_init->Exp;
                player.UpdateExp(CalculateExp(exp, GetLevel(), player.GetLevel()));
            }
        });
    }

    for (auto&[party_id, damage] : party_container)
    {
        max_party_damage = max_party_damage.second < damage ? std::make_pair(party_id, damage) : max_player_damage;
        long double exp_share = static_cast<long double>(damage) / static_cast<long double>(total_hostility);
        std::uint64_t exp = exp_share * m_init->Exp;

        World::ForParty(party_id, [&](Party& party)
        {
            party.AllotExp(CalculateExp(exp, GetLevel(), party.GetTopLevel()), {GetX(), GetY()});
        });
    }

    if(max_party_damage.second > max_player_damage.second)
    {
        World::ForParty(max_party_damage.first, [&](Party& party)
        {
            AllotLoot(party.GetID(), true);
        });
    }
    else
    {
        World::ForPlayer(max_player_damage.first, [&](Player& player)
        {
            AllotLoot(player.GetID());
        });
    }
    hostility_map.clear();
    total_hostility = 0;
}

void Monster::Die()
{
    AllotExpAndLoot();
}

void Monster::AllotLoot(std::unordered_set<id_t> loot_owners)
{
    std::vector<LootInfo> loot_vec;

    for(const auto& itemgroup : m_init->m_itemgroups)
    {
        auto* loot_itemgroup = ItemGroup::Find(itemgroup.m_index);
        if (!loot_itemgroup)
        {
            spdlog::error("ItemGroup of index {} not found.", itemgroup.m_index);
            continue;
        }

        for(std::uint32_t i = 0; i < itemgroup.m_number_of_rolls; i++)
        {
            const auto* rolled_group = loot_itemgroup->RollGroup();

            if(!rolled_group)
                continue;

            const auto loot_info_it = rolled_group->RollLoot();


            if(loot_info_it != rolled_group->m_loots_map.end())
                loot_vec.push_back(loot_info_it->second);
        }
    }

    for(auto& loot : loot_vec)
    {
        int new_loot_x = GetX() + (random::between(0, Loot::MAX_RANDOM_DISTANCE_FROM_THROWER * 2)) - Loot::MAX_RANDOM_DISTANCE_FROM_THROWER;
        int new_loot_y = GetY() + (random::between(0, Loot::MAX_RANDOM_DISTANCE_FROM_THROWER * 2)) - Loot::MAX_RANDOM_DISTANCE_FROM_THROWER;
        auto new_loot_ptr = std::make_shared<Loot>(loot, new_loot_x, new_loot_y, GetMap(), loot_owners);
        World::Add(new_loot_ptr);
    }
}