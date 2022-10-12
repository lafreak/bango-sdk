#include "BeheadableMonster.h"
#include "World.h"

using namespace bango::utils;

void BeheadableMonster::Die()
{
    SetGState(CGS_KNEE);
    m_death_time = time::now();
    World::Map(GetMap()).WriteInSight(this, bango::network::packet(S2C_ACTION, "db", GetID(), AT_KNEE));
}

void BeheadableMonster::Tick()
{
    if(IsGState(CGS_KNEE) && (time::now() - m_death_time).count() >= 10000)
    {
        SetGState(CGS_KO);
        World::Map(GetMap()).WriteInSight(this, bango::network::packet(S2C_ACTION, "db", GetID(), AT_DIE));
    }
}