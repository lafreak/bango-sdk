#include "Loot.h"

using namespace bango::processor;

void LootGroup::set(lisp::var param)
{
    switch (FindAttribute(param.pop()))
    {
        case A_INDEX: GetGroup().m_index = param.pop(); break;
        case IC_MONEY:
        case A_ITEM:
        {
            while (!param.null()) 
            {
                auto values_from_bracket = GetValuesFromBrackets(param);
                try
                {
                    AssignGroup(values_from_bracket);
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
            break;
        }
    }
}

void LootItemGroup::set(lisp::var param)
{
    switch (FindAttribute(param.pop()))
    {
        case A_INDEX: GetItemGroup().m_index = param.pop(); break;
        case A_GROUP:
        {
            while (!param.null()) 
            {
                auto values_from_bracket = GetValuesFromBrackets(param);
                try
                {
                    AssignItemGroup(values_from_bracket);
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
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

void LootItemGroup::AssignItemGroup(std::vector<uint32_t> values_from_bracket)
{
    if(values_from_bracket.size() != 2)
        throw std::runtime_error("Invalid amount of values in brackets for itemgroup index: " + std::to_string(GetItemGroup().m_index));

    auto& map = GetItemGroup().m_groups_map;
    auto max_key = map.GetMaxKey();

    GroupInfo group_info(values_from_bracket.at(1), values_from_bracket.at(0));
    ValidateGroupInfo(group_info);
    map.assign(max_key, values_from_bracket.at(0), group_info);

}

 void LootGroup::AssignGroup(std::vector<uint32_t> values_from_bracket)
 {
    static constexpr std::uint32_t geon_index = 31;

    auto& map = GetGroup().m_loots_map;
    auto max_key = map.GetMaxKey();

    LootInfo loot_info;

    if(values_from_bracket.size() == 2)
        loot_info = LootInfo(geon_index, values_from_bracket.at(0), values_from_bracket.at(1), 0);
    else if(values_from_bracket.size() == 3)
        loot_info = LootInfo(values_from_bracket.at(1), values_from_bracket.at(0), 1, values_from_bracket.at(2));
    else
        throw std::runtime_error("Invalid amount of values in brackets for group index: " + std::to_string(GetGroup().m_index));

    ValidateLootInfo(loot_info);
    map.assign(max_key, values_from_bracket.at(0), loot_info);
 }


void LootItemGroup::ValidateGroupInfo(GroupInfo current) const
{
    if(current.m_chance > 1000)
        throw std::runtime_error("Chance value is higher than 1000 for group index: " + std::to_string(current.m_index) + " in itemgroup:" + std::to_string(GetItemGroup().m_index));
    else if(current.m_chance <= GetItemGroup().m_groups_map.GetMaxKey())
        throw std::runtime_error("Chance value is lower than previous chance value for group index: " + std::to_string(current.m_index) + " in itemgroup:" + std::to_string(GetItemGroup().m_index));
}

void LootGroup::ValidateLootInfo(LootInfo current) const
{
    if(current.m_chance > 1000)
        throw std::runtime_error("Chance value is higher than 1000 for loot index: " + std::to_string(current.m_index) + " in group:" + std::to_string(GetGroup().m_index));
    else if(current.m_chance <= GetGroup().m_loots_map.GetMaxKey())
        throw std::runtime_error("Chance value is lower than previous chance value for loot index: " + std::to_string(current.m_index) + " in group:" + std::to_string(GetGroup().m_index));
}

