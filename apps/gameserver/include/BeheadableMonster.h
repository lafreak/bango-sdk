#pragma once
// Class representing beheadable monster.

#include "Monster.h"
#include <chrono>

class BeheadableMonster : public Monster
{
public:
    BeheadableMonster(const std::unique_ptr<InitMonster>& init, int x, int y, int map=0) : Monster(init, x, y) {}
    void Die() override;
    void Tick() override;

private:
    std::chrono::steady_clock::time_point m_death_time;
};