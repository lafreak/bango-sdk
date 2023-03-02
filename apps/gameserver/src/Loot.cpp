#include "Loot.h"

#include <bango/utils/random.h>

using namespace bango::processor;

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

    auto max_key = m_groups_map.GetMaxKey();

    GroupInfo group_info(group_index, values_from_bracket.at(0));
    ValidateGroupInfo(group_info);
    m_groups_map.assign(max_key, values_from_bracket.at(0), group_info);
}

 void Group::AssignGroup(std::vector<uint32_t> values_from_bracket)
 {
    static constexpr std::uint32_t geon_index = 31;

    auto max_key = m_loots_map.GetMaxKey();

    LootInfo loot_info;

    if(values_from_bracket.size() == 2)
        loot_info = LootInfo(geon_index, values_from_bracket.at(0), values_from_bracket.at(1), 0);
    else if(values_from_bracket.size() == 3)
        loot_info = LootInfo(values_from_bracket.at(1), values_from_bracket.at(0), 1, values_from_bracket.at(2));
    else if(values_from_bracket.size() == 4)
        loot_info = LootInfo(values_from_bracket.at(1), values_from_bracket.at(0), values_from_bracket.at(3), values_from_bracket.at(2));
    else
        throw std::runtime_error("Invalid amount of values in brackets for group index: " + std::to_string(m_index));

    ValidateLootInfo(loot_info);
    m_loots_map.assign(max_key, values_from_bracket.at(0), loot_info);
 }


void ItemGroup::ValidateGroupInfo(GroupInfo current) const
{
    if(current.m_chance > 1000)
        throw std::runtime_error("Chance value is higher than 1000 for group index: " + std::to_string(current.m_index) + " in itemgroup:" + std::to_string(m_index));
    else if(current.m_chance <= m_groups_map.GetMaxKey())
        throw std::runtime_error("Chance value is lower than previous chance value for group index: " + std::to_string(current.m_index) + " in itemgroup:" + std::to_string(m_index));
}

void Group::ValidateLootInfo(LootInfo current) const
{
    if(current.m_chance > 1000)
        throw std::runtime_error("Chance value is higher than 1000 for loot index: " + std::to_string(current.m_index) + " in group:" + std::to_string(m_index));
    else if(current.m_chance <= m_loots_map.GetMaxKey())
        throw std::runtime_error("Chance value is lower than previous chance value for loot index: " + std::to_string(current.m_index) + " in group:" + std::to_string(m_index));
}

LootInfo Group::RollLoot() const
{
    return m_loots_map[bango::utils::random::between(1, 1000)];
}

const Group* ItemGroup::RollGroup() const
{
    auto group_index = m_groups_map[bango::utils::random::between(1, 1000)].m_index;
    return Group::Find(group_index);
}