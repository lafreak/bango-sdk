#pragma once

#include <bango/space/quadtree.h>
#include <bango/network/packet.h>

#include <bango/utils/random.h>

#include <inix.h>

class Character : public bango::space::quad_entity
{
public:
    typedef std::uint32_t id_t;

private:
    std::uint32_t   m_id;
    std::uint8_t    m_type;
    std::uint16_t   m_dir;

protected: //TODO: Add method to inherit?
    std::uint32_t m_curhp=1, m_curmp=1;

public:
    int             m_z;
    std::uint8_t    m_map;

    std::uint16_t   m_n64GState;
    std::uint16_t   m_n64MState;
    std::uint16_t   m_n64GStateEx;
    std::uint16_t   m_n64MStateEx;

public:
    Character(std::uint8_t type) : m_type(type)
    {
        //TODO: Make ID pool for players.
        //BUG: Not thread safe.
        static id_t g_max_id=0;
        m_id = g_max_id++;
    }

    constexpr static std::uint8_t PLAYER    =0;
    constexpr static std::uint8_t MONSTER   =1;
    constexpr static std::uint8_t NPC       =2;
    constexpr static std::uint8_t LOOT      =3;

    id_t            GetID()     const { return m_id;    }
    std::uint8_t    GetType()   const { return m_type;  }
    std::uint8_t    GetMap()    const { return m_map;   }
    int             GetX()      const { return m_x;     }
    int             GetY()      const { return m_y;     }
    int             GetZ()      const { return m_z;     }
    std::uint16_t   GetDir()    const { return m_dir;   }

    virtual std::uint8_t    GetLevel()          const { return 1; }

    virtual std::uint8_t    GetAttackType()     const { return ATT_MEELE;  }
    virtual std::uint16_t   GetAttackSpeed()    const { return 0;          }

    virtual std::uint16_t   GetStrength()       const { return 0; }
    virtual std::uint16_t   GetHealth()         const { return 0; }
    virtual std::uint16_t   GetInteligence()    const { return 0; }
    virtual std::uint16_t   GetWisdom()         const { return 0; }
    virtual std::uint16_t   GetDexterity()      const { return 0; }

    virtual std::uint16_t   GetMinAttack()  const { return 1 + ((11 * GetStrength() - 80) / 30) + ((GetDexterity() - 5) / 11) + (7 * GetLevel() / 10); }
    virtual std::uint16_t   GetMaxAttack()  const { return ((8 * GetStrength() - 25) / 15) + (18 * GetDexterity() / 77) + GetLevel(); }
    virtual std::uint16_t   GetMinMagic()   const { return (7 * GetInteligence() - 20) / 12 + GetWisdom() / 7; }
    virtual std::uint16_t   GetMaxMagic()   const { return 7 * GetInteligence() / 12 + 14 * GetWisdom() / 45; }

    std::uint16_t           GetAttack()     const { return bango::utils::random::between(GetMinAttack(),  GetMaxAttack());  }
    std::uint16_t           GetMagic()      const { return bango::utils::random::between(GetMinMagic (),  GetMaxMagic ());  }

    virtual std::uint16_t   GetHit()        const { return GetDexterity() / 8 + 15 * GetStrength() / 54; }
    virtual std::uint16_t   GetDodge()      const { return GetDexterity() / 3; }
    virtual std::uint16_t   GetAbsorb()     const { return 0; }

    virtual std::uint16_t   GetDefense(std::uint8_t type)   const { return 0; }
    virtual std::uint16_t   GetResist (std::uint8_t type)   const;

    std::uint32_t   GetCurHP()  const { return m_curhp; }
    std::uint32_t   GetCurMP()  const { return m_curmp; }

    virtual std::uint32_t GetMaxHP() const { return 1; }
    virtual std::uint32_t GetMaxMP() const { return 1; }

    std::uint64_t   GetGState()     const { return 0; }
    std::uint64_t   GetMState()     const { return 0; }
    std::uint64_t   GetGStateEx()   const { return 0; }
    std::uint64_t   GetMStateEx()   const { return 0; }

    void AddGState(std::uint64_t n64GState)       { m_n64GState |= n64GState; }
    void AddMState(std::uint64_t n64MState)       { m_n64MState |= n64MState; }

    void AddGStateEx(std::uint64_t n64GStateEx)   { m_n64GStateEx |= n64GStateEx; }
    void AddMStateEx(std::uint64_t n64MStateEx)   { m_n64MStateEx |= n64MStateEx; }

    void SubGState(std::uint64_t n64GState)       { m_n64GState &= ~n64GState;}
    void SubMState(std::uint64_t n64MState)       { m_n64MState &= ~n64MState;}

    void SubGStateEx(std::uint64_t n64GStateEx)   { m_n64GStateEx &= ~n64GStateEx; }
    void SubMStateEx(std::uint64_t n64MStateEx)   { m_n64MStateEx &= ~n64MStateEx; }

    bool IsGState(std::uint64_t n64GState)        { return m_n64GState & n64GState; }
    bool IsMState(std::uint64_t n64MState)        { return m_n64MState & n64MState; }

    bool IsGStateEx(std::uint64_t n64GStateEx)    { return m_n64GStateEx & n64GStateEx; }
    bool IsMStateEx(std::uint64_t n64MStateEx)    { return m_n64MStateEx & n64MStateEx; }

    virtual bango::network::packet BuildAppearPacket(bool hero=false)   const { return bango::network::packet(); };
    virtual bango::network::packet BuildDisappearPacket()               const { return bango::network::packet(); };
    virtual bango::network::packet BuildMovePacket(std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop) const { return bango::network::packet(); };

    void LookAt(Character* character) { LookAt(character->m_x, character->m_y); }
    void LookAt(int x, int y) { SetDirection(x - m_x, y - m_y); }
    void SetDirection(std::int8_t delta_x, std::int8_t delta_y);

    bool            CheckHit(Character* target, int bonus=0);
    std::int64_t    GetFinalDamage(Character* attacker, std::int64_t damage, bool magical=false);

    virtual void Tick() {}
};