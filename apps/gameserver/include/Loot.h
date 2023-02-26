#include "Inventory.h"
#include "Character.h"
#include "spdlog/spdlog.h"

#include <bango/processor/db.h>
#include <bango/utils/interval_map.h>
#include <inix/attributes.h>


struct LootInfo
{
    LootInfo() = default;
    LootInfo(std::uint32_t index, std::uint32_t chance, std::uint32_t amount, std::uint32_t prefix)
        : m_index(index)
        , m_chance(chance)
        , m_amount(amount)
        , m_prefix(prefix)
    {}

    constexpr bool operator==(const LootInfo&) const = default;

    std::uint32_t m_index;
    std::uint32_t m_chance;
    std::uint32_t m_amount;
    std::uint32_t m_prefix;
};

struct GroupInfo
{
    GroupInfo() = default;
    GroupInfo(std::uint32_t index, std::uint32_t chance)
        : m_index(index)
        , m_chance(chance)
    {}

    constexpr bool operator==(const GroupInfo&) const = default;


    std::uint32_t m_index;
    std::uint32_t m_chance;
};

struct Group
{
    Group() = default;
    std::uint32_t m_index;
    bango::utils::interval_map<LootInfo> m_loots_map;
};

struct ItemGroup
{
    ItemGroup() = default;
    std::uint32_t m_index;
    bango::utils::interval_map<GroupInfo> m_groups_map;
};


class LootGroup : public bango::processor::db_object<LootGroup>
{
    Group m_group;
public:

    unsigned int index() const { return m_group.m_loots_map.GetMaxKey() != 0 ? m_group.m_index : 0; }

    void ValidateLootInfo(LootInfo current) const;
    void AssignGroup(std::vector<uint32_t> values_from_bracket);
    Group& GetGroup() { return m_group; }
    Group GetGroup() const { return m_group; }

    virtual void set(bango::processor::lisp::var param) override;
};


class LootItemGroup : public bango::processor::db_object<LootItemGroup>
{
    ItemGroup m_itemgroup;
public:
    
    unsigned int index() const { return m_itemgroup.m_groups_map.GetMaxKey() != 0 ? m_itemgroup.m_index : 0; }

    void ValidateGroupInfo(GroupInfo current) const;
    void AssignItemGroup(std::vector<uint32_t> values_from_bracket);
    ItemGroup& GetItemGroup()  { return m_itemgroup; }
    ItemGroup GetItemGroup() const { return m_itemgroup; }

    virtual void set(bango::processor::lisp::var param) override;
};

std::vector<std::uint32_t> GetValuesFromBrackets(bango::processor::lisp::var& param);
