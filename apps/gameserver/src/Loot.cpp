#include "Loot.h"

#include <bango/utils/random.h>

using namespace bango::processor;
using namespace bango::network;
using namespace bango::utils;

void Group::set(lisp::var param)
{
    switch (FindAttribute(param.pop()))
    {
        case A_INDEX: m_index = param.pop(); break;
        case IC_MONEY:
        case A_ITEM:
        {
            while (!param.null()) 
            {
                auto values_from_bracket = GetValuesFromBrackets(param);
                AssignGroup(values_from_bracket);
            }
            break;
        }
    }
}

void ItemGroup::set(lisp::var param)
{
    switch (FindAttribute(param.pop()))
    {
        case A_INDEX: m_index = param.pop(); break;
        case A_GROUP:
        {
            while (!param.null()) 
            {
                auto values_from_bracket = GetValuesFromBrackets(param);
                AssignItemGroup(values_from_bracket);
            }
            break;
        }
    }
}


std::vector<std::uint32_t> GetValuesFromBrackets(lisp::var& param)
{
    std::vector<std::uint32_t> values_from_bracket;
    lisp::var subList = param.car();

    while (!subList.null()) 
    {
        std::uint32_t value = subList.car();
        values_from_bracket.push_back(value);
        subList = subList.cdr();
    }

    param = param.cdr();


    return values_from_bracket;
}

void ItemGroup::AssignItemGroup(std::vector<uint32_t> values_from_bracket)
{
    if(values_from_bracket.size() != 2)
        throw std::runtime_error("Invalid amount of values in brackets for itemgroup index: " + std::to_string(m_index));

    std::uint32_t group_index = values_from_bracket.at(1);

    if(!Group::Find(group_index))
        throw std::runtime_error("Group index: " + std::to_string(group_index) + " not found");

    auto new_chance = values_from_bracket.at(0);

    if(GetMaxMapKey() == 0 && new_chance == 0)
    {
        spdlog::warn("Ignoring group_index {} for itemgroup {} due to 0 chance", group_index, index());
        return;
    }

    GroupInfo group_info(group_index, new_chance);
    ValidateGroupInfo(group_info);
    m_groups_map.insert({new_chance, group_info});
}

 void Group::AssignGroup(std::vector<uint32_t> values_from_bracket)
 {
    if(values_from_bracket.size() < 2 || values_from_bracket.size() > 4)
        throw std::runtime_error("Invalid amount of values in brackets for group index: " + std::to_string(m_index));

    static constexpr std::uint32_t geon_index = 31;

    auto item_index = values_from_bracket.size() == 2 ? geon_index : values_from_bracket.at(1);
    auto item_chance = values_from_bracket.at(0);
    auto item_amount = values_from_bracket.size() == 2 ? values_from_bracket.at(1) : (values_from_bracket.size() == 3 ? 1 : values_from_bracket.at(3));
    auto item_prefix = values_from_bracket.size() == 2 ? 0 : values_from_bracket.at(2);

    if(GetMaxMapKey() == 0 && item_chance == 0)
    {
        spdlog::warn("Ignoring item_index {} for group {} due to 0 chance", item_index, index());
        return;
    }

    LootInfo loot_info(item_index, item_chance, item_amount, item_prefix);

    ValidateLootInfo(loot_info);
    m_loots_map.insert({item_chance, loot_info});
 }


void ItemGroup::ValidateGroupInfo(GroupInfo current) const
{
    if(current.m_chance > 1000)
        throw std::runtime_error("Chance value is higher than 1000 for group index: " + std::to_string(current.m_index) + " in itemgroup:" + std::to_string(m_index));
    else if(current.m_chance <= GetMaxMapKey())
        throw std::runtime_error("Chance value is lower than previous chance value for group index: " + std::to_string(current.m_index) + " in itemgroup:" + std::to_string(m_index));
}

void Group::ValidateLootInfo(LootInfo current) const
{
    if(current.m_chance > 1000)
        throw std::runtime_error("Chance value is higher than 1000 for loot index: " + std::to_string(current.m_index) + " in group:" + std::to_string(m_index));
    else if(current.m_chance <= GetMaxMapKey())
        throw std::runtime_error("Chance value is lower than previous chance value for loot index: " + std::to_string(current.m_index) + " in group:" + std::to_string(m_index));
}

std::map<std::uint32_t, LootInfo>::const_iterator Group::RollLoot() const
{
    return m_loots_map.upper_bound(random::between(1, 1000));
}

const Group* ItemGroup::RollGroup() const
{
    auto rolled_group_it = m_groups_map.upper_bound(random::between(1, 1000));

    //If nothing was chosen
    if(rolled_group_it == m_groups_map.end())
        return nullptr;

    auto group_index = rolled_group_it->second.m_index;

    const auto* group = Group::Find(group_index);
    if(!group)
    {
        spdlog::error("Group index: " + std::to_string(group_index) + " not found");
        return nullptr;
    }

    return group;
}

Loot::Loot(LootInfo loot_info, int x, int y, int map)
    : Character(Character::LOOT)
{
    m_appear_time = time::now();
    m_x = x;
    m_y = y;
    m_map = map;

    memset(&m_item_info, 0, sizeof(m_item_info));
    m_item_info.Index = loot_info.m_index;
    m_item_info.Num = loot_info.m_amount;
    m_item_info.Prefix = loot_info.m_prefix;
}

packet Loot::BuildAppearPacket(bool hero) const
{
    packet p(S2C_CREATEITEM);
    p << GetItemIndex() << GetID() << GetX() << GetY() << static_cast<std::int32_t>(-1) << GetAmount();
    return p;
}

packet Loot::BuildDisappearPacket() const
{
    return packet(S2C_REMOVEITEM, "d", GetID());
}

bool Loot::IsExpired() const
{
    return (time::now() - m_appear_time).count() >= Loot::DISAPPEAR_TIME;
}

void Loot::Tick()
{
    
}