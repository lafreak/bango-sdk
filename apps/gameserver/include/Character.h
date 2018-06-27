#pragma once

#include <bango/space/quadtree.h>
#include <bango/network/packet.h>

class Character : public bango::space::quad_entity
{
    std::uint32_t   m_id;
    std::uint8_t    m_type;
    std::uint16_t   m_dir;

public:
    int             m_z;
    std::uint8_t    m_map;

public:
    Character(std::uint8_t type) : m_type(type)
    {
        //BUG: Not thread safe.
        static std::uint32_t g_max_id=0;
        m_id = g_max_id++;
    }

    constexpr static std::uint8_t PLAYER    =0;
    constexpr static std::uint8_t MONSTER   =1;
    constexpr static std::uint8_t NPC       =2;
    constexpr static std::uint8_t LOOT      =3;

    std::uint32_t   GetID()     const { return m_id; }
    std::uint8_t    GetType()   const { return m_type; }
    std::uint8_t    GetMap()    const { return m_map; }
    int             GetX()      const { return m_x; }
    int             GetY()      const { return m_y; }
    int             GetZ()      const { return m_z; }
    std::uint16_t   GetDir()    const { return m_dir; }

    virtual bango::network::packet BuildAppearPacket(bool hero=false)   const { return bango::network::packet(); };
    virtual bango::network::packet BuildDisappearPacket()               const { return bango::network::packet(); };
    virtual bango::network::packet BuildMovePacket(std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop) const { return bango::network::packet(); };

    void LookAt(int x, int y) { SetDirection(x - m_x, y - m_y); }
    void SetDirection(std::int8_t delta_x, std::int8_t delta_y);

    virtual void Tick() {}
};