#include "BeheadableMonster.h"

#include <bango/network/packet.h>
#include <bango/utils/time.h>
#include <inix.h>

using namespace bango::network;
using namespace bango::utils;

void BeheadableMonster::Die()
{
    Monster::Die();
    SetGState(CGS_KNEE);
    SetDeathTime(time::now());
    WriteInSight(packet(S2C_ACTION, "db", GetID(), AT_KNEE));
}

void BeheadableMonster::Tick()
{
    if(IsGState(CGS_KNEE) && (time::now() - GetDeathTime()).count() >= 1000)//10000) // FIXME: Change back when behead is added.
    {
        SetGState(CGS_KO);
        WriteInSight(packet(S2C_ACTION, "db", GetID(), AT_DIE));
    }
}

bango::utils::time::point BeheadableMonster::GetDeathTime() const
{
    return m_death_time;
}
void BeheadableMonster::SetDeathTime(bango::utils::time::point death_time)
{
    m_death_time = death_time;
}