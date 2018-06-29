#pragma once

#include "Character.h"

#include <inix.h>

class Monster : public Character
{
    std::uint16_t m_index;
    std::uint8_t m_race=1;//non-beheadable
    std::uint8_t m_level=50;

public:
    Monster(std::uint16_t index, int x, int y, int map=0)
        : Character(Character::MONSTER)
    {
        m_index = index;
        m_x = x;
        m_y = y;
        m_map = map;
    }

    std::uint16_t GetIndex() const { return m_index; }
    std::uint8_t GetRace() const { return m_race; }
    std::uint8_t GetLevel() const { return m_level; }

    bango::network::packet BuildAppearPacket(bool hero=false) const override;
    bango::network::packet BuildDisappearPacket() const override;
    bango::network::packet BuildMovePacket(std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop) const override;

    void Tick() override;
};