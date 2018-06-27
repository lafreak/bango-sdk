#pragma once

#include "Character.h"
#include "User.h"
#include "Inventory.h"

#include "CommandDispatcher.h"

#include <inix.h>

class Player : public Character, public User, public Inventory
{
    PLAYERINFO m_data;
    std::string m_name;

public:
    Player(const bango::network::taco_client_t& client)
        : User(client), Character(Character::PLAYER) {}

    void OnLoadPlayer(bango::network::packet& p)
    {
        p >> m_data >> m_name >> m_x >> m_y;
        m_z = m_data.Z;
        m_map = m_data.Map;

        write(S2C_PROPERTY, "bsbwwwwwwddwwwwwbIwwwwwwbbbbbd",
            0, //Grade
            "\0", //GuildName
            0, //GRole
            GetContribute(),
            GetStrength(),
            GetHealth(),
            GetInteligence(),
            GetWisdom(),
            GetDexterity(),
            GetCurHP(),
            GetCurHP(), //MaxHP
            GetCurMP(),
            GetCurMP(), //MaxMP
            1, //Hit
            2, //Dodge
            3, //Defense
            4, //Absorb
            GetExp(),
            5, //MinAttack
            6, //MaxAttack
            7, //MinMagic
            8, //MaxMagic
            GetPUPoint(),
            GetSUPoint(),
            9, //ResistFire
            10, //ResistIce
            11, //ResistLitning
            12, //ResistCurse
            13, //ResistPalsy
            GetRage());

        short time = 1200;
        write(S2C_ANS_LOAD, "wdd", time, GetX(), GetY());
    }

    void OnRest(bango::network::packet& p)
    {
        auto action = p.pop<char>();
        std::cout << "On Rest " << (int)action << std::endl;
    }

    void OnLoadItems(bango::network::packet& p)
    {
        unsigned short count = p.pop<unsigned short>();

        for (unsigned short i = 0; i < count; i++)
        {
            auto info = p.pop<ITEMINFO>();
            Inventory::Insert(info);
        }

        write((Inventory)*this);
    }

    // Network I/O Endpoints
    void OnConnected();
    void OnDisconnected();
    void OnStart(bango::network::packet& p);
    void OnRestart(bango::network::packet& p);
    void OnExit(bango::network::packet& p);
    void OnMove(bango::network::packet& p, bool end);
    void OnChatting(bango::network::packet& p);
    void OnPutOnItem(bango::network::packet& p);
    void OnPutOffItem(bango::network::packet& p);
    void OnUseItem(bango::network::packet& p);
    void OnTrashItem(bango::network::packet& p);

    // Command Endpoints
    void OnGetItem(CommandDispatcher::Token& token);

    // Map Endpoints
    void OnCharacterAppear(Character * subject);
    void OnCharacterDisappear(Character * subject);
    void OnCharacterMove(Character * subject, std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop);

/*
    const Item::Ptr PutOnItem(bango::network::packet& p)
    {
        auto local = p.pop<unsigned int>();
        auto item = FindByLocalID(local);
        if (!item)
            return nullptr;

        if (item->GetInit().LimitClass != PC_ALL && item->GetInit().LimitClass != GetClass())
            return nullptr;

        if (item->GetInit().LimitLevel > GetLevel())
            return nullptr;

        return PutOn(local);
    }
*/
    bool CanLogout() const { return true; }

    bango::network::packet BuildAppearPacket(bool hero=false) const override
    {
        bango::network::packet p(S2C_CREATEPLAYER);

        p   << GetID()
            << GetName()
            << GetClass(hero) 
            << GetX() 
            << GetY() 
            << GetZ() 
            << GetDir() 
            << GetGState()
            << GetEquipment()
            << GetFace() 
            << GetHair() 
            << GetMState() 
            << "\0" << "\0" // GuildClass & GuildName
            << GetGID() 
            << GetFlag() 
            << GetFlagItem()
            << GetHonorGrade() 
            << GetHonorOption() 
            << GetGStateEx() 
            << GetMStateEx();

        // Unknown
        p << (std::int8_t)0 << (std::int32_t)0 << (std::int32_t)0 << (std::int8_t)0;

        return p;
    }
    
    bango::network::packet BuildDisappearPacket() const override
    {
        return bango::network::packet(S2C_REMOVEPLAYER, "d", GetID());
    }

    bango::network::packet BuildMovePacket(std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop) const override
    {
        return bango::network::packet(stop ? S2C_MOVEPLAYER_END : S2C_MOVEPLAYER_ON, "dbbb", GetID(), delta_x, delta_y, delta_z);
    }

    std::int32_t        GetPID()                    const { return m_data.PlayerID; }
    const std::string&  GetName()                   const { return m_name; }
    std::uint8_t        GetClass(bool hero=false)   const { return hero ? (m_data.Class | GAME_HERO) : m_data.Class; }
    std::uint8_t        GetJob()                    const { return m_data.Job; }
    std::uint8_t        GetLevel()                  const { return m_data.Level; }
    std::uint16_t       GetStrength()               const { return m_data.Strength; }
    std::uint16_t       GetHealth()                 const { return m_data.Health; }
    std::uint16_t       GetInteligence()            const { return m_data.Inteligence; }
    std::uint16_t       GetWisdom()                 const { return m_data.Wisdom; }
    std::uint16_t       GetDexterity()              const { return m_data.Dexterity; }
    std::uint32_t       GetCurHP()                  const { return m_data.CurHP; }
    std::uint32_t       GetCurMP()                  const { return m_data.CurMP; }
    std::uint64_t       GetExp()                    const { return m_data.Exp; }
    std::uint16_t       GetPUPoint()                const { return m_data.PUPoint; }
    std::uint16_t       GetSUPoint()                const { return m_data.SUPoint; }
    std::uint16_t       GetContribute()             const { return m_data.Contribute; }
    std::uint32_t       GetRage()                   const { return m_data.Rage; }
    std::int32_t        GetX()                      const { return m_x; }
    std::int32_t        GetY()                      const { return m_y; }
    std::int32_t        GetZ()                      const { return m_z; }
    std::uint8_t        GetMap()                    const { return m_data.Map; }
    std::uint8_t        GetFace()                   const { return m_data.Face; }
    std::uint8_t        GetHair()                   const { return m_data.Hair; }

    // Not implemented
    std::uint64_t       GetGState()     const { return 0; }
    std::uint64_t       GetMState()     const { return 0; }
    std::uint32_t       GetGID()        const { return 0; }
    std::uint8_t        GetFlag()       const { return 0; }
    std::uint32_t       GetFlagItem()   const { return 0; }
    std::uint32_t       GetHonorGrade() const { return 0; }
    std::uint32_t       GetHonorOption()const { return 0; }
    std::uint64_t       GetGStateEx()   const { return 0; }
    std::uint64_t       GetMStateEx()   const { return 0; }

    void InsertItem(unsigned short index, unsigned int num=1);
    bool TrashItem(unsigned int local);

    void Tick() override;
};