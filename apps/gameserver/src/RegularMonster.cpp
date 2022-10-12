#include "RegularMonster.h"
#include "World.h"

void RegularMonster::Die()
{
    SetGState(CGS_KO);
    World::Map(GetMap()).WriteInSight(this, bango::network::packet(S2C_ACTION, "db", GetID(), AT_DIE));
}

void RegularMonster::Tick()
{
    
}