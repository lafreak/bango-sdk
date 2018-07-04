#pragma once

#include "Character.h"
#include "User.h"
#include "Inventory.h"

#include "CommandDispatcher.h"

#include <inix.h>

#include <bango/utils/time.h>

class Player : public Character, public User, public Inventory
{
    PLAYERINFO m_data;
    std::string m_name;

    //! Teleportation coordinates waiting for Z coordinate from client.
    int m_teleport_x=0;
    int m_teleport_y=0;

    bango::utils::time::point 
        m_last_attack;

public:
    Player(const bango::network::taco_client_t& client)
        : User(client), Character(Character::PLAYER) {}

    // Network I/O Endpoints
    void OnConnected        ();
    void OnDisconnected     ();
    void OnStart            (bango::network::packet& p);
    void OnRestart          (bango::network::packet& p);
    void OnExit             (bango::network::packet& p);
    void OnMove             (bango::network::packet& p, bool end);
    void OnLoadPlayer       (bango::network::packet& p);
    void OnLoadItems        (bango::network::packet& p);
    void OnLoadFinish       ();
    void OnRest             (bango::network::packet& p);
    void OnChatting         (bango::network::packet& p);
    void OnPutOnItem        (bango::network::packet& p);
    void OnPutOffItem       (bango::network::packet& p);
    void OnUseItem          (bango::network::packet& p);
    void OnTrashItem        (bango::network::packet& p);
    void OnTeleportAnswer   (bango::network::packet& p);
    void OnUpdateProperty   (bango::network::packet& p);
    void OnPlayerAnimation  (bango::network::packet& p);
    void OnAttack           (bango::network::packet& p);

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

    std::uint8_t        GetLevel()                  const override { return m_data.Level; }

    std::uint8_t        GetAttackType()             const override { return GetClass() == PC_ARCHER ? 1 : 0; }
    std::uint16_t       GetAttackSpeed()            const override { return Inventory::GetAddAttackSpeed(); }

    std::uint16_t       GetBaseStrength()           const { return m_data.Strength;     }
    std::uint16_t       GetBaseHealth()             const { return m_data.Health;       }
    std::uint16_t       GetBaseInteligence()        const { return m_data.Inteligence;  }
    std::uint16_t       GetBaseWisdom()             const { return m_data.Wisdom;       }
    std::uint16_t       GetBaseDexterity()          const { return m_data.Dexterity;    }

    std::uint16_t       GetStrength()               const override { return GetBaseStrength()       + Inventory::GetAddStrength();      }
    std::uint16_t       GetHealth()                 const override { return GetBaseHealth()         + Inventory::GetAddHealth();        }
    std::uint16_t       GetInteligence()            const override { return GetBaseInteligence()    + Inventory::GetAddInteligence();   }
    std::uint16_t       GetWisdom()                 const override { return GetBaseWisdom()         + Inventory::GetAddWisdom();        }
    std::uint16_t       GetDexterity()              const override { return GetBaseDexterity()      + Inventory::GetAddDexterity();     }

    std::uint16_t       GetMinAttack()              const override { return Character::GetMinAttack()   + Inventory::GetAddMinAttack(); }
    std::uint16_t       GetMaxAttack()              const override { return Character::GetMaxAttack()   + Inventory::GetAddMaxAttack(); }
    std::uint16_t       GetMinMagic()               const override { return Character::GetMinMagic()    + Inventory::GetAddMinMagic();  }
    std::uint16_t       GetMaxMagic()               const override { return Character::GetMaxMagic()    + Inventory::GetAddMaxMagic();  }

    std::uint16_t       GetHit()                    const override { return Character::GetHit()     + Inventory::GetAddHit();       }
    std::uint16_t       GetDodge()                  const override { return Character::GetDodge()   + Inventory::GetAddDodge();     }
    std::uint16_t       GetAbsorb()                 const override { return                           Inventory::GetAddAbsorb();    }

    std::uint16_t       GetDefense  (std::uint8_t type=ATT_MEELE)   const override { return                               Inventory::GetAddDefense();   }
    std::uint16_t       GetResist   (std::uint8_t type)             const override { return Character::GetResist(type)  + Inventory::GetAddResist(type);}

    std::uint32_t       GetMaxHP()                  const override;
    std::uint32_t       GetMaxMP()                  const override;

    std::uint64_t       GetExp()                    const { return m_data.Exp; }
    std::uint16_t       GetPUPoint()                const { return m_data.PUPoint; }
    std::uint16_t       GetSUPoint()                const { return m_data.SUPoint; }
    std::uint16_t       GetContribute()             const { return m_data.Contribute; }
    std::uint32_t       GetRage()                   const { return m_data.Rage; }
    std::uint8_t        GetFace()                   const { return m_data.Face; }
    std::uint8_t        GetHair()                   const { return m_data.Hair; }

    // Not implemented
    std::uint32_t       GetGID()        const { return 0; }
    std::uint8_t        GetFlag()       const { return 0; }
    std::uint32_t       GetFlagItem()   const { return 0; }
    std::uint32_t       GetHonorGrade() const { return 0; }
    std::uint32_t       GetHonorOption()const { return 0; }

    bool CanLogout() const { return true; }
    void SendInventoryProperty();
    void SendProperty(std::uint8_t kind);

    void InsertItem(unsigned short index, unsigned int num=1);
    bool TrashItem(unsigned int local);
    void Teleport(int x, int y, int z=0);

    std::uint16_t   GetReqPU(std::uint8_t* stats);

    void Tick() override;
};