#include "BeheadableMonster.h"
#include "World.h"

using namespace std::chrono;

void BeheadableMonster::Die()
{
    SetGState(CGS_KNEE);
    m_death_time = steady_clock::now();
    World::Map(GetMap()).WriteInSight(this, bango::network::packet(S2C_ACTION, "db", GetID(), AT_KNEE));
}

void BeheadableMonster::Tick()
{
    if(IsGState(CGS_KNEE) && duration_cast<seconds>(steady_clock::now() - m_death_time).count() >= 10)
    {
        SetGState(CGS_KO);
        World::Map(GetMap()).WriteInSight(this, bango::network::packet(S2C_ACTION, "db", GetID(), AT_DIE));
    }
}