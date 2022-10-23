#include "BeheadableMonster.h"

#include <bango/network/packet.h>
#include <bango/utils/time.h>
#include <inix.h>

using namespace bango::network;
using namespace bango::utils;

void BeheadableMonster::Die()
{
    SetGState(CGS_KNEE);
    m_death_time = time::now();
    WriteInSight(packet(S2C_ACTION, "db", GetID(), AT_KNEE));
}

void BeheadableMonster::Tick()
{
    if(IsGState(CGS_KNEE) && (time::now() - m_death_time).count() >= 10000)
    {
        SetGState(CGS_KO);
        WriteInSight(packet(S2C_ACTION, "db", GetID(), AT_DIE));
    }
}