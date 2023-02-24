#include "Inventory.h"
#include "Character.h"
#include "spdlog/spdlog.h"

#include <bango/processor/db.h>
#include <inix/attributes.h>

struct LootInfo
{
    LootInfo(std::uint32_t index, std::uint32_t chance, std::uint32_t amount, std::uint32_t prefix)
        : m_index(index)
        , m_chance(chance)
        , m_amount(amount)
        , m_prefix(prefix)
    {}
    std::uint32_t m_index;
    std::uint32_t m_chance;
    std::uint32_t m_amount;
    std::uint32_t m_prefix;
};

struct GroupInfo
{
    GroupInfo(std::uint32_t index, std::uint32_t chance)
        : m_index(index)
        , m_chance(chance)
    {}
    std::uint32_t m_index;
    std::uint32_t m_chance;
};

struct Group
{
    std::uint32_t m_index;
    std::vector<LootInfo> m_loots;
};

struct ItemGroup 
{
    std::uint32_t m_index;
    std::vector<GroupInfo> m_groups;
};


struct LootGroup : public bango::processor::db_object<LootGroup>
{
    Group m_group;

    unsigned int index() const { return m_group.m_loots.empty() ? 0 : m_group.m_index; }

    virtual void set(bango::processor::lisp::var param) override;
};


struct LootItemGroup : public bango::processor::db_object<LootItemGroup>
{
    ItemGroup m_itemgroup;
    
    unsigned int index() const { return m_itemgroup.m_groups.empty() ? 0 : m_itemgroup.m_index; }

    virtual void set(bango::processor::lisp::var param) override;
};

std::vector<std::uint32_t> GetValuesFromBrackets(bango::processor::lisp::var& param);

template<typename T>
void SetChances(std::vector<T>& vec,const std::vector<std::uint32_t>& chances_from_config)
{
    for(std::size_t i = 1; i < vec.size(); i++)
    {
        if(chances_from_config[i] <= chances_from_config[i-1])
            throw std::runtime_error("Previous chance is larger or equal than current chance for index: " + std::to_string(vec[i - 1].m_index));
        
        vec[i].m_chance -= chances_from_config[i-1];
    }
}

template<typename T>
std::vector<std::uint32_t> GetConfigChancesFromVector(const std::vector<T>& vec)
{
    std::vector<std::uint32_t> chances;
    for(auto& item : vec)
    {
        if(item.m_chance > 1000)
            throw std::runtime_error("Chance is larger than 1000 for index: " + std::to_string(item.m_index));

        chances.push_back(item.m_chance);
    }

    return chances;
}

template<typename DbObject>
void CalculateChances()
{
    using namespace bango::processor;

    for(auto& [index, group] : DbObject::DB())
    {
        if constexpr (std::is_same_v<DbObject, LootGroup>)
        {
            auto chances_from_config = GetConfigChancesFromVector(group->m_group.m_loots);
            SetChances(group->m_group.m_loots, chances_from_config);
        }
        else if constexpr (std::is_same_v<DbObject, LootItemGroup>)
        {
            auto chances_from_config = GetConfigChancesFromVector(group->m_itemgroup.m_groups);
            SetChances(group->m_itemgroup.m_groups, chances_from_config);
        }
        else
            throw std::runtime_error("Wrong template argument");

    }
}
