#include "RegularMonster.h"
#include "World.h"

#include <bango/network/packet.h>
#include <bango/utils/time.h>
#include <inix.h>

using namespace bango::network;

void RegularMonster::Die()
{
    SetGState(CGS_KO);
    SetDeathTime(bango::utils::time::now());
    WriteInSight(packet(S2C_ACTION, "db", GetID(), AT_DIE));
}

void RegularMonster::Tick()
{
    
}