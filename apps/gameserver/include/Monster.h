#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Character.h"

#include <bango/processor/db.h>
#include <bango/utils/time.h>
#include <inix.h>

struct MonsterItemGroup
{
    MonsterItemGroup(std::uint32_t rolls, std::uint32_t index)
        : m_index(index)
        , m_rolls(rolls)
    {}

    std::uint32_t m_index;
    std::uint32_t m_rolls;
};

struct InitMonster : public bango::processor::db_object<InitMonster>
{
    std::vector<MonsterItemGroup> m_itemgroups;
    unsigned int
        Index=0, Race=0, Level=1, AI=0, Range=0, CloseSight=0, FarSight=0, Exp=0,
        Strength=0, Health=0, Inteligence=0, Wisdom=0, Dexterity=0,
        HP=0, MP=0, AttackSpeed=0, Hit=0, Dodge=0, Size=0, AttackType=0,
        MinAttack=0, MaxAttack=0, MinMagic=0, MaxMagic=0, 
        Absorb=0, WalkSpeed=0, RunSpeed=0;

    unsigned int Resist [5] ={0,};
    unsigned int Defense[2] ={0,};

    unsigned int index() const { return Index; }

    virtual void set(bango::processor::lisp::var param) override;
};

class Monster : public Character
{
    const std::unique_ptr<InitMonster>& m_init;
    std::unordered_map<id_t, std::uint64_t> hostility_map;
    std::uint64_t total_hostility;
    void DistributeExp();

public:
    Monster(const std::unique_ptr<InitMonster>& init, int x, int y, int map=0);
    ~Monster();

    void RestoreInitialState(int new_x, int new_y);
    virtual void ReceiveDamage(id_t id, std::uint32_t damage) override;

    std::uint16_t   GetIndex()      const { return m_init->Index; }
    std::uint8_t    GetRace()       const { return m_init->Race; }

    std::uint8_t    GetLevel()      const override { return m_init->Level; }

    std::uint8_t    GetAttackType() const override { return m_init->AttackType;  }
    std::uint16_t   GetAttackSpeed()const override { return m_init->AttackSpeed; }

    std::uint16_t   GetStrength()   const override { return m_init->Strength;       }
    std::uint16_t   GetHealth()     const override { return m_init->Health;         }
    std::uint16_t   GetInteligence()const override { return m_init->Inteligence;    }
    std::uint16_t   GetWisdom()     const override { return m_init->Wisdom;         }
    std::uint16_t   GetDexterity()  const override { return m_init->Dexterity;      }

    std::uint16_t   GetMinAttack()  const override { return Character::GetMinAttack()   + m_init->MinAttack; }
    std::uint16_t   GetMaxAttack()  const override { return Character::GetMaxAttack()   + m_init->MaxAttack; }
    std::uint16_t   GetMinMagic()   const override { return Character::GetMinMagic()    + m_init->MinMagic; }
    std::uint16_t   GetMaxMagic()   const override { return Character::GetMaxMagic()    + m_init->MaxMagic; }

    std::uint16_t   GetHit()        const override { return Character::GetHit()     + m_init->Hit;  }
    std::uint16_t   GetDodge()      const override { return Character::GetDodge()   + m_init->Dodge;}
    std::uint16_t   GetAbsorb()     const override { return                           m_init->Absorb; }

    std::uint16_t   GetDefense  (std::uint8_t type=ATT_MEELE)   const override { return m_init->Defense[type];  }
    std::uint16_t   GetResist   (std::uint8_t type)             const override { return m_init->Resist [type];  }

                                                        //TODO: Level based?                            
    std::uint32_t   GetMaxHP()      const override { return (52 * GetLevel() / 3)   + 115               + 2 * GetHealth() * GetHealth() / /*deno*/10 + m_init->HP; }
    std::uint32_t   GetMaxMP()      const override { return (8  * GetLevel()    )   + 140 + GetWisdom() + 2 * GetWisdom() * GetWisdom() / /*deno*/10 + m_init->MP; }


    bango::network::packet BuildAppearPacket(bool hero=false) const override;
    bango::network::packet BuildDisappearPacket() const override;
    bango::network::packet BuildMovePacket(std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop) const override;
    static void Summon(std::uint32_t index, std::int32_t x, std::int32_t y, std::int32_t map);
    static std::shared_ptr<Monster> CreateMonster(std::uint32_t index, std::int32_t x, std::int32_t y, std::int32_t map);
protected:
    virtual void Die() override;
};