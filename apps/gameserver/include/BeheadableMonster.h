#pragma once
// Class representing beheadable monster.

#include <memory>

#include "Monster.h"
#include <bango/utils/time.h>

class BeheadableMonster : public Monster
{
public:
    BeheadableMonster(const std::unique_ptr<InitMonster>& init, int x, int y, int map=0) : Monster(init, x, y) {}
    void Die() override;
    void Tick() override;

private:
    bango::utils::time::point m_death_time;
};