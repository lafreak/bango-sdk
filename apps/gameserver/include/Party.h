#pragma once

#include <cstdint>
#include <list>
#include <unordered_set>
#include <mutex>

#include <bango/network/writable.h>
#include <bango/space/quadtree.h>

/*TODO list: (future commits)
update player position(Tick function in Player?)
Add MSG_OFFLINE_OUTOFRANGE to Player::OnAskParty and OnAskPartyAnswer
*/

class Player;

class Party
{
    std::list<Player*> m_members_list;
    mutable std::recursive_mutex m_rmtx_list;
    id_t m_id;
    std::uint8_t m_top_level{0};
public:
    static constexpr std::uint8_t MAX_PARTY_SIZE = 8;
    static constexpr std::uint8_t MIN_PARTY_SIZE = 2;
    static constexpr std::uint16_t MAX_EXP_RECEIVE_RANGE = 640;
    static constexpr std::uint8_t MAX_LEVEL_DIFFERENCE_TO_RECEIVE_EXP = 28;

    Party() = delete;
    Party(Player* leader, Player* player);
    ~Party();

    //! Gets index of the party
    id_t GetID()                               const;

    //! Gets size of a party.
    std::uint8_t GetSize()                     const;

    //! Gets leader of the party.
    Player*      GetLeader()                   const;

    //! Checks if party size is between min and max.
    bool         IsValid()                     const;
    
    //! Checks if given player is party leader.
    bool         IsLeader(const Player* player)const;

    //! Checks if party is full.
    bool         IsFull()                      const;

    //! Checks if party is empty.
    bool         IsEmpty()                     const;

    //! Sends party list with properties to all party members.
    void         SendPartyInfo()               const;

    //! Gets highest level of the party
    std::uint8_t GetTopLevel()                 const;

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

    //! Sets new highest level of the party
    void         UpdateTopLevel();

    //! Distribute exp for all players in MAP_SIGHT range
    void         AllotExp(std::uint64_t exp, bango::space::point p);

    //! Get all party members PID
    std::unordered_set<id_t> GetMembersPID() const;
};