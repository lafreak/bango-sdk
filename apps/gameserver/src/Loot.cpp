#include "Loot.h"

using namespace bango::processor;

void LootGroup::set(lisp::var param)
{
    switch (FindAttribute(param.pop()))
    {
        case A_INDEX: m_group.m_index = param.pop(); break;
        case IC_MONEY:
        {
            static constexpr std::uint32_t geon_index = 31;

            while (!param.null()) 
            {
                auto temp_vector = GetValuesFromBrackets(param);

                if(temp_vector.size() == 2)
                    m_group.m_loots.push_back(LootInfo(geon_index, temp_vector.at(0), temp_vector.at(1), 0));
            }
            break;
        }
        case A_ITEM:
        {
            while (!param.null()) 
            {
                auto temp_vector = GetValuesFromBrackets(param);

                if(temp_vector.size() == 3)
                    m_group.m_loots.push_back(LootInfo(temp_vector.at(1),temp_vector.at(0), 1, temp_vector.at(2)));
            }
            break;
        }
    }
}

void LootItemGroup::set(lisp::var param)
{
    switch (FindAttribute(param.pop()))
    {
        case A_INDEX: m_itemgroup.m_index = param.pop(); break;
        case A_GROUP:
        {
            while (!param.null()) 
            {
                auto temp_vector = GetValuesFromBrackets(param);

                if(temp_vector.size() == 2)
                    m_itemgroup.m_groups.push_back(GroupInfo(temp_vector.at(1), temp_vector.at(0)));
            }
            break;
        }
    }
}


std::vector<std::uint32_t> GetValuesFromBrackets(lisp::var& param)
{
    std::vector<std::uint32_t> temp_vector;
    lisp::var subList = param.car();

    while (!subList.null()) 
    {
        std::uint32_t value = subList.car();
        temp_vector.push_back(value);
        subList = subList.cdr();
    }

    param = param.cdr();

    return temp_vector;
}
