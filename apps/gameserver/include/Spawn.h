#pragma once

#include <vector>
#include <memory>

#include "Monster.h"

#include <bango/processor/db.h>
#include <bango/utils/time.h>

//TODO:
//Monster might not exist in InitMonster -> spawn shouldn't be created
//Some monsters do not spawn with error cannot insert into quad with error: not in boundary

struct GenMonster : public bango::processor::db_object<GenMonster>
{
    struct RectXY
    {
        std::int32_t X1 = 0, Y1 = 0, X2 = 0, Y2 = 0;
        std::int32_t GetRandomX() const;
        std::int32_t GetRandomY() const;

    };
    std::int32_t MonsterIndex = 0, Map = 0, Area = 0, Amount = 0, SpawnCycle = 0;
    RectXY Rect{};

    unsigned int index() const;
    virtual void set(bango::processor::lisp::var param) override;
};

class Spawn
{
public:

    Spawn(const std::unique_ptr<GenMonster>& init);

    void Tick();
private:
    void RespawnOnWorld(const std::shared_ptr<Monster> monster);
    void CreateSpawn();
    void SetNextSpawnCycle();

    std::int32_t                         GetArea()           const;
    std::int32_t                         GetMonsterIndex()   const;
    std::int32_t                         GetMap()            const;
    std::int32_t                         GetAmount()         const;
    std::int32_t                         GetSpawnCycle()     const;
    std::int32_t                         GetRandomX()        const;
    std::int32_t                         GetRandomY()        const;
    GenMonster::RectXY                   GetRect()          const;

    const std::unique_ptr<GenMonster>& m_init;
    std::vector<std::shared_ptr<Monster>> m_area_monsters;
    bango::utils::time::point m_next_spawn_cycle;
};