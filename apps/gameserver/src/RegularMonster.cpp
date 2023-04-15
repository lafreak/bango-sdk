#include "RegularMonster.h"

#include <bango/network/packet.h>
#include <bango/utils/time.h>
#include <inix.h>

using namespace bango::network;

void RegularMonster::Die()
{
    Monster::Die();
    AddGState(CGS_KO);
    WriteInSight(packet(S2C_ACTION, "db", GetID(), AT_DIE));
}

void RegularMonster::Tick()
{
    
}