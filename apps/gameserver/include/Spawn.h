#pragma once

#include <vector>
#include <memory>

#include "Monster.h"

#include <inix.h>
#include <bango/processor/db.h>
#include <bango/utils/random.h>

//TODO:
//thread-safe
//Tick should be every spawncycle, not every second.

struct GenMonster : public bango::processor::db_object<GenMonster>
{
    struct RectXY
    {
        std::uint32_t X1 = 0, Y1 = 0, X2 = 0, Y2 = 0;
        std::uint32_t GetRandomX() const;
        std::uint32_t GetRandomY() const;

    };
    std::uint32_t MonsterIndex = 0, Map = 0, Index = 0, Amount = 0, SpawnCycle = 0;
    RectXY Rect;

    unsigned int index() const;
    virtual void set(bango::processor::lisp::var param) override;
};

class Spawn
{
public:

    Spawn(const std::unique_ptr<GenMonster>& init);

    void Tick();

    std::uint32_t                         GetIndex()          const;
    std::uint32_t                         GetMonsterIndex()   const;
    std::uint32_t                         GetMap()            const;
    std::uint32_t                         GetAmount()         const;
    std::uint32_t                         GetSpawnCycle()     const;
    std::uint32_t                         GetRandomX()        const;
    std::uint32_t                         GetRandomY()        const;
    GenMonster::RectXY                    GetRect()           const;
    std::vector<std::shared_ptr<Monster>> GetMonsterVector()  const;
    const std::unique_ptr<GenMonster>&    GetInitMonster()    const;

private:
    void RespawnOnWorld(std::shared_ptr<Monster> monster);
    bool IsMonsterReadyToRespawn(std::shared_ptr<Monster> monster);
    std::shared_ptr<Monster> CreateMonster();
    const std::unique_ptr<GenMonster>& m_init;
    std::vector<std::shared_ptr<Monster>> m_area_monsters;
};