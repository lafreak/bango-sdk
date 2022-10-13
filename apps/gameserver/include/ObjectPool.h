#pragma once

#include "Monster.h"
#include <stack>

class ObjectPool
{
    ObjectPool() = default;
    ObjectPool& operator=(const ObjectPool&) = delete;
    ObjectPool(const ObjectPool&) = delete;

public:
    static ObjectPool& GetInstance()
    {
        static ObjectPool instance;
        return instance;
    }
    
    Monster* GetRegularMonster()
    {
        if(regular_monster_pool.empty())
            return nullptr;

        Monster* monster = regular_monster_pool.top();
        regular_monster_pool.pop();
        return monster;
    }

    Monster* GetBeheadableMonster()
    {
        if(beheadable_monster_pool.empty())
            return nullptr;

        Monster* monster = beheadable_monster_pool.top();
        beheadable_monster_pool.pop();
        return monster;
    }

    void AddRegularMonster(Monster* monster)
    {
        regular_monster_pool.push(monster);
    }

    void AddBeheadableMonster(Monster* monster)
    {
        beheadable_monster_pool.push(monster);
    }
private:

    std::stack<Monster*> regular_monster_pool;
    std::stack<Monster*> beheadable_monster_pool;
};