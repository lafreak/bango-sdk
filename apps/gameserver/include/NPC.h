#pragma once

#include "Character.h"

#include <bango/processor/db.h>
#include <inix.h>

struct InitNPC : public bango::processor::db_object<InitNPC>
{
    unsigned int
        Index,
        Kind,
        Shape,
        HTML,
        Map,
        X, Y, Z,
        DirX, DirY;

    unsigned int index() const { return Index; }

    virtual void set(bango::processor::lisp::var param) override
    {
        switch (FindAttribute(param.pop()))
        {
            case A_INDEX:       Index       = param.pop(); break;
            case A_KIND:        Kind        = param.pop(); break;
            case A_SHAPE:       Shape       = param.pop(); break;
            case A_HTML:        HTML        = param.pop(); break;
            case A_MAP:         Map         = param.pop(); break;

            case A_XY:          X           = param.pop();
                                Y           = param.pop();
                                Z           = param.pop();
                                break;

            case A_DIR:         DirX        = param.pop();
                                DirY        = param.pop();
                                break;
        }
    }
};

class NPC : public Character
{
    const InitNPC* m_init;
public:
    NPC(const InitNPC* init) 
        : Character(Character::NPC), m_init(init)
    {
        m_x =   init->X;
        m_y =   init->Y;
        m_z =   init->Z;
        m_map = init->Map;

        LookAt(init->DirX, init->DirY);
    }

    unsigned short  GetIndex() const { return m_init->Index; }
    unsigned char   GetShape() const { return m_init->Shape; }

    bango::network::packet BuildAppearPacket(bool hero=false) const override
    {
        bango::network::packet p(S2C_CREATENPC);
        p   << GetID()
            << GetIndex()
            << GetShape()
            << GetX()
            << GetY()
            << GetZ()
            << GetDir()
            << (std::int64_t) 0 // GState
            << (std::uint32_t) 0 // FlagItem
            << (std::uint32_t) 0 // Unknown
            ;
        return p;
    }

    bango::network::packet BuildDisappearPacket() const override
    {
        return bango::network::packet(S2C_REMOVENPC, "d", GetID());
    }

    void Tick() override;
};