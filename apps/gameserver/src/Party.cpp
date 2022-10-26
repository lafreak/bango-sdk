#include "Party.h"

#include <exception>
#include <mutex>

#include "spdlog/spdlog.h"

#include "Player.h"

#include <bango/network/packet.h>
#include <inix.h>

using namespace bango::network;

Party::Party(Player* leader, Player* player)
{
    spdlog::trace("Party constructor between: {} and {}", leader->GetName(), player->GetName());
    AddMember(leader);
    AddMember(player);
}

Party::~Party()
{
    spdlog::trace("Party destructor");
}

void Party::SendPartyInfo() const
{
    packet p(S2C_PARTYINFO);
    p.push<unsigned char>(GetSize());
    for (auto* player : m_members_list)
    {
        p.push<unsigned int>(player->GetID());
        p.push_str(player->GetName());
        p.push<unsigned char>(player->GetClass());
        p.push<unsigned char>(player->GetLevel());
        p.push<unsigned short>(player->GetCurHP());
        p.push<unsigned short>(player->GetMaxHP());
    }
    WriteToAll(p);
}

void Party::WriteToAll(const packet& p) const
{
    std::lock_guard<std::recursive_mutex> guard(m_rmtx_list);
    for (auto* player : m_members_list)
        player->write(p);
}

bool Party::AddMember(Player* player)
{
    std::lock_guard<std::recursive_mutex> guard(m_rmtx_list);
    //player->ResetPartyInviterID(); // TODO: Move out

    if (IsFull())
    {
        player->write(S2C_MESSAGE, "b", MSG_PARTYISFULL);
        return false;
    }

    if (!IsEmpty())
    {
        packet p(S2C_MESSAGEV);
        p.push<unsigned char>(MSG_JOINEDINPARTY);
        p.push<unsigned int>(player->GetID());
        p.push_str(player->GetName().c_str());
        p.push<unsigned char>(player->GetClass());
        p.push<unsigned char>(player->GetLevel());
        p.push<unsigned short>(player->GetCurHP());
        p.push<unsigned short>(player->GetMaxHP());
        WriteToAll(p);
    }

    m_members_list.push_back(player);
    SendPartyInfo();
    return true;
}

void Party::RemoveMember(Player* player, bool is_kicked)
{
    std::lock_guard<std::recursive_mutex> guard(m_rmtx_list);
    if (std::find(m_members_list.begin(), m_members_list.end(), player) == m_members_list.end())
        throw std::runtime_error("Finding player to remove from party failed.");

    bool was_leaver_party_leader = GetLeader() == player;//player->IsPartyLeader();
    m_members_list.remove(player);

    if (is_kicked)
    {
        player->write(S2C_MESSAGE, "b", MSG_EXILEDFROMPARTY);
        WriteToAll(packet(S2C_MESSAGEV, "bd", MSG_EXILEDFROMPARTY, player->GetID()));
    }
    else
    {
        player->write(S2C_MESSAGE, "b", MSG_LEFTPARTY);
        WriteToAll(packet(S2C_MESSAGEV, "bd", MSG_LEFTPARTY, player->GetID()));
    }

    if (GetSize() == 1)
    {
        GetLeader()->write(S2C_MESSAGE, "b", MSG_ENDPARTY);
    }
    else if (GetSize() > 1)
    {
        SendPartyInfo();
        if (was_leaver_party_leader)
            GetLeader()->write(S2C_MESSAGE, "b", MSG_BECOMEPARTYHEAD);
    }
}

std::uint8_t Party::GetSize() const
{
    return m_members_list.size(); 
}

Player* Party::GetLeader() const
{
    return !IsEmpty() ? m_members_list.front() : nullptr; 
}

bool Party::IsLeader(const Player* player) const
{
    return GetLeader() == player;
}

bool Party::IsValid() const
{
    return GetSize() >= MIN_PARTY_SIZE;
}

bool Party::IsFull() const
{
    return GetSize() >= MAX_PARTY_SIZE; 
}

bool Party::IsEmpty() const
{
    return m_members_list.empty(); 
}