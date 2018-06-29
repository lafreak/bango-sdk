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

    //! Teleportation coordinates waiting for Z coordinate from client.
    int m_teleport_x=0;
    int m_teleport_y=0;

public:
    Player(const bango::network::taco_client_t& client)
        : User(client), Character(Character::PLAYER) {}

    // Network I/O Endpoints
    void OnConnected();
    void OnDisconnected();
    void OnStart(bango::network::packet& p);
    void OnRestart(bango::network::packet& p);
    void OnExit(bango::network::packet& p);
    void OnMove(bango::network::packet& p, bool end);
    void OnLoadPlayer(bango::network::packet& p);
    void OnLoadItems(bango::network::packet& p);
    void OnRest(bango::network::packet& p);
    void OnChatting(bango::network::packet& p);
    void OnPutOnItem(bango::network::packet& p);
    void OnPutOffItem(bango::network::packet& p);
    void OnUseItem(bango::network::packet& p);
    void OnTrashItem(bango::network::packet& p);
    void OnTeleportAnswer(bango::network::packet& p);

    // Command Endpoints
    void OnGetItem(CommandDispatcher::Token& token);
    void OnMoveTo(CommandDispatcher::Token& token);

    // Map Endpoints
    void OnCharacterAppear(Character * subject, bool hero);
    void OnCharacterDisappear(Character * subject);
    void OnCharacterMove(Character * subject, std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop);

    bango::network::packet BuildAppearPacket(bool hero=false) const override;
    bango::network::packet BuildDisappearPacket() const override;
    bango::network::packet BuildMovePacket(std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop) const override;

    std::int32_t        GetPID()                    const { return m_data.PlayerID; }
    const std::string&  GetName()                   const { return m_name; }
    std::uint8_t        GetClass(bool hero=false)   const { return hero ? (m_data.Class | GAME_HERO) : m_data.Class; }
    std::uint8_t        GetJob()                    const { return m_data.Job; }
    //
    std::uint8_t        GetLevel()                  const { return m_data.Level; }
    std::uint16_t       GetStrength()               const { return m_data.Strength; }
    std::uint16_t       GetHealth()                 const { return m_data.Health; }
    std::uint16_t       GetInteligence()            const { return m_data.Inteligence; }
    std::uint16_t       GetWisdom()                 const { return m_data.Wisdom; }
    std::uint16_t       GetDexterity()              const { return m_data.Dexterity; }
    std::uint32_t       GetCurHP()                  const { return m_data.CurHP; }
    std::uint32_t       GetCurMP()                  const { return m_data.CurMP; }
    //
    std::uint64_t       GetExp()                    const { return m_data.Exp; }
    std::uint16_t       GetPUPoint()                const { return m_data.PUPoint; }
    std::uint16_t       GetSUPoint()                const { return m_data.SUPoint; }
    std::uint16_t       GetContribute()             const { return m_data.Contribute; }
    std::uint32_t       GetRage()                   const { return m_data.Rage; }
    //std::int32_t        GetX()                      const { return m_x; }
    //std::int32_t        GetY()                      const { return m_y; }
    //std::int32_t        GetZ()                      const { return m_z; }
    //std::uint8_t        GetMap()                    const { return m_data.Map; }
    std::uint8_t        GetFace()                   const { return m_data.Face; }
    std::uint8_t        GetHair()                   const { return m_data.Hair; }

    // Not implemented
    std::uint32_t       GetGID()        const { return 0; }
    std::uint8_t        GetFlag()       const { return 0; }
    std::uint32_t       GetFlagItem()   const { return 0; }
    std::uint32_t       GetHonorGrade() const { return 0; }
    std::uint32_t       GetHonorOption()const { return 0; }

    bool CanLogout() const { return true; }

    void InsertItem(unsigned short index, unsigned int num=1);
    bool TrashItem(unsigned int local);
    void Teleport(int x, int y, int z=0);

    void Tick() override;
};