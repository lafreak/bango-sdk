#pragma once

#include <inix/protocol.h>
#include <inix/common.h>
#include <inix/structures.h>
#include <bango/network.h>

#include "Inventory.h"

using namespace bango::network;

class Player : public component
{
    using component::component;

    PLAYERINFO m_data;

    std::string m_name;

    Inventory m_inventory;

public:

    const char* id() const override
    {
        return "PlayerComponent";
    }

    void OnLoadPlayer   (packet& p);
    void OnLoadItems    (packet& p);
    void GameStart      (packet& p);
    void GameRestart    ();

    bool CanLogout() const { return true; }


    void                SetAccountID (int value) { m_data.AccountID = value; }

    int                 GetAccountID()  const { return m_data.AccountID; }
    int                 GetPlayerID()   const { return m_data.PlayerID; }
    const std::string&  GetName()       const { return m_name; }
    unsigned char       GetClass()      const { return m_data.Class; }
    unsigned char       GetJob()        const { return m_data.Job; }
    unsigned char       GetLevel()      const { return m_data.Level; }
    unsigned short      GetStrength()   const { return m_data.Strength; }
    unsigned short      GetHealth()     const { return m_data.Health; }
    unsigned short      GetInteligence()const { return m_data.Inteligence; }
    unsigned short      GetWisdom()     const { return m_data.Wisdom; }
    unsigned short      GetDexterity()  const { return m_data.Dexterity; }
    unsigned int        GetCurHP()      const { return m_data.CurHP; }
    unsigned int        GetCurMP()      const { return m_data.CurMP; }
    unsigned long       GetExp()        const { return m_data.Exp; }
    unsigned short      GetPUPoint()    const { return m_data.PUPoint; }
    unsigned short      GetSUPoint()    const { return m_data.SUPoint; }
    unsigned short      GetContribute() const { return m_data.Contribute; }
    unsigned int        GetRage()       const { return m_data.Rage; }
    unsigned int        GetX()          const { return m_data.X; }
    unsigned int        GetY()          const { return m_data.Y; }
    unsigned int        GetZ()          const { return m_data.Z; }
    unsigned char       GetFace()       const { return m_data.Face; }
    unsigned char       GetHair()       const { return m_data.Hair; }
};
