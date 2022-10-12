#pragma once
// Class representing non-beheadable monster.

#include "Monster.h"

class NormalMonster : public Monster
{
public:
    NormalMonster(const std::unique_ptr<InitMonster>& init, int x, int y, int map=0) : Monster(init, x, y) {}
    void Die() override;
    void Tick() override;
};