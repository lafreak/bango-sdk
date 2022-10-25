#pragma once

#include <cstdint>
#include <list>
#include <mutex>

#include <bango/network/writable.h>

/*TODO list: (future commits)
update player position(Tick function in Player?)
Add MSG_OFFLINE_OUTOFRANGE to Player::OnAskParty and OnAskPartyAnswer
*/

class Player;

class Party
{
    std::list<Player*> m_members_list;
    mutable std::recursive_mutex m_rmtx_list;
public:
    static constexpr uint8_t MAX_PARTY_SIZE = 8;
    static constexpr uint8_t MIN_PARTY_SIZE = 2;

    Party() = delete;
    Party(Player* leader, Player* player);
    ~Party();

    std::uint8_t GetSize()                     const;
    Player*      GetLeader()                   const;
    bool         IsValid()                     const;
    bool         IsFull()                      const;
    bool         IsEmpty()                     const;
    void         SendPartyInfo()               const;
    void         WriteToAll(const bango::network::packet& p)   const;
    bool         AddMember(Player* player);
    void         RemoveMember(Player* player, bool is_kicked = false);
};