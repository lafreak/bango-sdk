#pragma once
// Class representing non-beheadable monster.

#include <memory>

#include "Monster.h"

class RegularMonster : public Monster
{
public:
    RegularMonster(const std::unique_ptr<InitMonster>& init, int x, int y, int map=0) : Monster(init, x, y, map) {}
    void Die() override;
    void Tick() override;
};