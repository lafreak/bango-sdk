#pragma once

#include <cstdint>
#include <memory>

#include "Character.h"
#include "User.h"
#include "Inventory.h"
#include "Party.h"
#include "Skill.h"

#include "CommandDispatcher.h"

#include <inix.h>

#include <bango/utils/time.h>
#include <bango/network/server.h>

class Player : public Character, public User
{
    PLAYERINFO m_data;
    std::string m_name;

    Inventory m_inventory;
    SkillManager m_skills;
    std::shared_ptr<Party> m_party;
    int m_party_inviter_id=0;
    id_t m_party_id=0;

    //! Teleportation coordinates waiting for Z coordinate from client.
    int m_teleport_x=0;
    int m_teleport_y=0;

    bango::utils::time::point m_last_attack = bango::utils::time::now();
    bango::utils::time::point m_last_save = bango::utils::time::now();

    static constexpr std::uint32_t SAVE_ALL_PROPERTY_INTERVAL = 60'000;
public:
    Player(const bango::network::taco_client_t& client);
    ~Player();

    // Network I/O Endpoints
    void OnConnected            ();
    void OnDisconnected         ();
    void OnStart                (bango::network::packet& p);
    void OnRestart              (bango::network::packet& p);
    void OnExit                 (bango::network::packet& p);
    void OnMove                 (bango::network::packet& p, bool end);
    void OnLoadPlayer           (bango::network::packet& p);
    void OnLoadItems            (bango::network::packet& p);
    void OnLoadSkills           (bango::network::packet& p);
    void OnLoadFinish           ();
    void OnRest                 (bango::network::packet& p);
    void OnChatting             (bango::network::packet& p);
    void OnPutOnItem            (bango::network::packet& p);
    void OnPutOffItem           (bango::network::packet& p);
    void OnUseItem              (bango::network::packet& p);
    void OnTrashItem            (bango::network::packet& p);
    void OnTeleportAnswer       (bango::network::packet& p);
    void OnUpdateProperty       (bango::network::packet& p);
    void OnPlayerAnimation      (bango::network::packet& p);
    void OnAttack               (bango::network::packet& p);
    void OnAskParty             (bango::network::packet& p);
    void OnAskPartyAnswer       (bango::network::packet& p);
    void OnExileParty           (bango::network::packet& p);
    void OnLeaveParty           (bango::network::packet& p);
    void OnPickUpItem           (bango::network::packet& p);
    void OnSkillUp              (bango::network::packet& p);
    void OnLearnSkill           (bango::network::packet& p);
    void OnSkill                (bango::network::packet& p);
    void OnPreSkill             (bango::network::packet& p);

    // Command Endpoints
    void OnGetItem(CommandDispatcher::Token& token);
    void OnMove2(CommandDispatcher::Token& token);
    void OnMoveTo(CommandDispatcher::Token& token);

    // Map Endpoints
    void OnCharacterAppear(Character& subject, bool hero);
    void OnCharacterDisappear(Character& subject);
    void OnCharacterMove(Character& subject, std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop);

    bango::network::packet BuildAppearPacket(bool hero=false) const override;
    bango::network::packet BuildDisappearPacket() const override;
    bango::network::packet BuildMovePacket(std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop) const override;

    std::int32_t        GetPID()                    const { return m_data.PlayerID; }
    const std::string&  GetName()                   const { return m_name; }
    std::uint8_t        GetClass(bool hero=false)   const { return hero ? (m_data.Class | GAME_HERO) : m_data.Class; }
    std::uint8_t        GetJob()                    const { return m_data.Job; }

    std::uint8_t        GetLevel()                  const override { return m_data.Level; }

    std::uint8_t        GetAttackType()             const override { return GetClass() == PC_ARCHER ? 1 : 0; }
    std::uint16_t       GetAttackSpeed()            const override { return m_inventory.GetAttackSpeed(); }

    std::uint16_t       GetBaseStrength()           const { return m_data.Strength;     }
    std::uint16_t       GetBaseHealth()             const { return m_data.Health;       }
    std::uint16_t       GetBaseInteligence()        const { return m_data.Inteligence;  }
    std::uint16_t       GetBaseWisdom()             const { return m_data.Wisdom;       }
    std::uint16_t       GetBaseDexterity()          const { return m_data.Dexterity;    }

    std::uint16_t       GetStrength()               const override { return GetBaseStrength()       + m_inventory.GetStrength();      }
    std::uint16_t       GetHealth()                 const override { return GetBaseHealth()         + m_inventory.GetHealth();        }
    std::uint16_t       GetInteligence()            const override { return GetBaseInteligence()    + m_inventory.GetInteligence();   }
    std::uint16_t       GetWisdom()                 const override { return GetBaseWisdom()         + m_inventory.GetWisdom();        }
    std::uint16_t       GetDexterity()              const override { return GetBaseDexterity()      + m_inventory.GetDexterity();     }

    std::uint16_t       GetMinAttack()              const override { return Character::GetMinAttack()   + m_inventory.GetMinAttack(); }
    std::uint16_t       GetMaxAttack()              const override { return Character::GetMaxAttack()   + m_inventory.GetMaxAttack(); }
    std::uint16_t       GetMinMagic()               const override { return Character::GetMinMagic()    + m_inventory.GetMinMagic();  }
    std::uint16_t       GetMaxMagic()               const override { return Character::GetMaxMagic()    + m_inventory.GetMaxMagic();  }

    std::uint16_t       GetHit()                    const override { return Character::GetHit()     + m_inventory.GetHit();       }
    std::uint16_t       GetDodge()                  const override { return Character::GetDodge()   + m_inventory.GetDodge();     }
    std::uint16_t       GetAbsorb()                 const override { return                           m_inventory.GetAbsorb();    }

    std::uint16_t       GetDefense  (std::uint8_t type=ATT_MEELE)   const override { return                               m_inventory.GetDefense();   }
    std::uint16_t       GetResist   (std::uint8_t type)             const override { return Character::GetResist(type)  + m_inventory.GetResist(type);}

    std::uint32_t       GetMaxHP()                  const override;
    std::uint32_t       GetMaxMP()                  const override;

    std::uint64_t       GetExp()                    const { return m_data.Exp; }
    std::uint16_t       GetPUPoint()                const { return m_data.PUPoint; }
    std::uint16_t       GetSUPoint()                const { return m_data.SUPoint; }
    std::uint16_t       GetContribute()             const { return m_data.Contribute; }
    std::uint32_t       GetRage()                   const { return m_data.Rage; }
    std::uint8_t        GetFace()                   const { return m_data.Face; }
    std::uint8_t        GetHair()                   const { return m_data.Hair; }

    Inventory&          GetInventory()                    { return m_inventory; }

    // Not implemented
    std::uint32_t       GetGID()        const { return 0; }
    std::uint8_t        GetFlag()       const { return 0; }
    std::uint32_t       GetFlagItem()   const { return 0; }
    std::uint32_t       GetHonorGrade() const { return 0; }
    std::uint32_t       GetHonorOption()const { return 0; }

    virtual void UpdatePropertyPoint(std::uint8_t kind, std::int64_t value) override;

    bool CanLogout() const { return true; }
    void SendInventoryProperty() const;
    void SendProperty(std::uint8_t kind, std::int64_t amount = 0) const;
    void SaveAllProperty() const;

    void InsertItem(unsigned short index, unsigned int num=1);
    bool TrashItem(unsigned int local);
    void Teleport(int x, int y, int z=0);

    void SetPartyInviterID(id_t id)                       { m_party_inviter_id = id; }
    void ResetPartyInviterID()                            { m_party_inviter_id = 0; }
    id_t  GetPartyInviterID()                  const      { return m_party_inviter_id; }
    
    //! Leave from party.
    //! Display proper message if forced by leader via kick. 
    void LeaveParty(bool is_kicked = false);

    void BanFromParty(id_t banned_player_id);
    void SetParty(const std::shared_ptr<Party>& party)          { m_party = party; m_party_id = party->GetID(); }
    void ResetParty()                                           { m_party = nullptr; m_party_id = 0; }
    bool HasParty()                                     const   { return m_party != nullptr; }
    bool IsPartyLeader()                                const   { return HasParty() && GetParty()->IsLeader(this); }
    const std::shared_ptr<Party>& GetParty()            const   { return m_party; }
    id_t GetPartyID()                                   const   { return m_party_id; }

    bool CanLearnSkill(std::uint8_t index) const;

    std::uint16_t   GetReqPU(std::uint8_t* stats);

    void Tick() override;
    void Die() override;
    void ResetStates() override;

    void UpdateExp(std::int64_t amount);
    bool CanReceiveExp();  // TODO: Make use of it
    std::uint64_t CalculateExp(std::uint64_t exp, std::uint8_t monster_level);
    void LevelUp();
};