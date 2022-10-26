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
    static constexpr std::uint8_t MAX_PARTY_SIZE = 8;
    static constexpr std::uint8_t MIN_PARTY_SIZE = 2;

    Party() = delete;
    Party(Player* leader, Player* player);
    ~Party();

    //! Gets size of a party.
    std::uint8_t GetSize()                     const;

    //! Gets leader of the party.
    Player*      GetLeader()                   const;

    //! Checks if party size is between min and max.
    bool         IsValid()                     const;

    //! Checks if party is full.
    bool         IsFull()                      const;

    //! Checks if party is empty.
    bool         IsEmpty()                     const;

    //! Sends party list with properties to all party members.
    void         SendPartyInfo()               const;

    //! Sends packet to all party members.
    //! Thread safe.
    void         WriteToAll(const bango::network::packet& p)   const;

    //! Tries to add player to the party.
    //! Returns bool whether player has been successfully added.
    //! Thread safe.
    bool         AddMember(Player* player);

    //! Removes player from the party.
    //! Throws an exception if player could not be found inside the party.
    //! Thread safe.
    void         RemoveMember(Player* player, bool is_kicked = false);
};