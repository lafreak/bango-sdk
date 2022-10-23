#include "Monster.h"
#include "World.h"
#include "RegularMonster.h"
#include "BeheadableMonster.h"

#include <iostream>

using namespace bango::network;

Monster::~Monster()
{
    std::cout << "Monster ptr [" << (int*)this << "] destructor" << std::endl;
}

packet Monster::BuildAppearPacket(bool hero) const
{
    //                                wdddwddIIsbdsIIb
    return packet(S2C_CREATEMONSTER, "wdddwddIIsbdsIIb", 
        GetIndex(),
        GetID(),
        GetX(),
        GetY(),
        GetDir(),
        GetCurHP(),//200,//GetCurHP()
        GetMaxHP(),//200,//GetMaxHP()
        GetGState(),
        GetMState(),
        "\0",
        GetRace(),//TODO: | (hero ? GAME_HERO : 0),
        0, //gid
        "\0",
        GetGStateEx(),
        GetMStateEx(),
        GetLevel()
        );
}

packet Monster::BuildDisappearPacket() const
{
    return packet(S2C_REMOVEMONSTER, "d", GetID());
}

packet Monster::BuildMovePacket(std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop) const
{
    return packet();
}
// World::Add(new RegularMonster(InitMonster::DB().at(index), player->GetX()+10, player->GetY()+10, player->GetMap()));

void Monster::CreateMonster(uint32_t index, int32_t x, int32_t y, int32_t map)
{
    switch(InitMonster::DB().at(index)->Race)
    {
        case MR_NOTMAGUNI:
        {
            World::Add(std::make_shared<RegularMonster>(InitMonster::DB().at(index), x, y, map));
            break;
        }
        case MR_MAGUNI:
        { 
            World::Add(std::make_shared<BeheadableMonster>(InitMonster::DB().at(index), x, y, map));
            break;
        }
        default:
        {
            World::Add(std::make_shared<RegularMonster>(InitMonster::DB().at(index), x, y, map));
            break;
        }
    }
}