#pragma once

#include <map>

#include "Inventory.h"
#include "Character.h"
#include "spdlog/spdlog.h"

#include <bango/processor/db.h>
#include <bango/utils/time.h>
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

struct Group : public bango::processor::db_object<Group>
{
    std::uint32_t m_index;
    std::map<std::uint32_t, LootInfo> m_loots_map;

    std::uint32_t GetMaxMapKey() const { return m_loots_map.empty() ? 0 : m_loots_map.rbegin()->first;}

    unsigned int index() const { return m_index; }

    void ValidateLootInfo(LootInfo current) const;
    void AssignGroup(std::vector<uint32_t> values_from_bracket);
    std::map<std::uint32_t, LootInfo>::const_iterator RollLoot() const;

    virtual void set(bango::processor::lisp::var param) override;
};


struct ItemGroup : public bango::processor::db_object<ItemGroup>
{
    std::uint32_t m_index;
    std::map<std::uint32_t, GroupInfo> m_groups_map;

    std::uint32_t GetMaxMapKey() const { return m_groups_map.empty() ? 0 : m_groups_map.rbegin()->first;}
    
    unsigned int index() const { return m_index; }

    void ValidateGroupInfo(GroupInfo current) const;
    void AssignItemGroup(std::vector<uint32_t> values_from_bracket);
    const Group* RollGroup() const;

    virtual void set(bango::processor::lisp::var param) override;
};

std::vector<std::uint32_t> GetValuesFromBrackets(bango::processor::lisp::var& param);


class Loot : public Character
{
    ITEMINFO m_item_info;
    bango::utils::time::point m_appear_time;
public:
    static constexpr std::uint32_t DISAPPEAR_TIME = 180000;
    static constexpr std::uint32_t PRIORITY_TIME = 120000;
    static constexpr std::int32_t MAX_DISTANCE_FROM_TARGET = 20;
    static constexpr std::int32_t MIN_DISTANCE_FROM_TARGET = -20;

    Loot(LootInfo loot_info, int x, int y, int map);

    std::uint16_t GetItemIndex() const { return m_item_info.Index; }
    std::uint32_t GetAmount() const { return m_item_info.Num; }
    std::uint8_t GetPrefix() const { return m_item_info.Prefix; }
    const ITEMINFO& GetItemInfo() const { return m_item_info; }

    bango::utils::time::point GetAppearTime() const;
    void                      ResetAppearTime();

    bango::network::packet BuildAppearPacket(bool hero=false) const override;

    bango::network::packet BuildDisappearPacket() const override;

    void Tick() override;
    void Die() override {}
};