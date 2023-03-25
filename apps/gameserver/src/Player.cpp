#include "Player.h"

#include <string>
#include <utility>

#include "spdlog/spdlog.h"

#include "Socket.h"
#include "World.h"
#include "Monster.h"

#include <bango/utils/random.h>
#include <bango/network/packet.h>
#include <inix.h>

using namespace bango::network;
using namespace bango::utils;

Player::Player(const bango::network::taco_client_t& client) : User(client), Character(Character::PLAYER)
{
    spdlog::trace("Player constructor id: {}", GetID());
}

Player::~Player()
{
    spdlog::trace("Player destructor id: {}", GetID());
}

void Player::OnConnected()
{
    assign(User::CAN_REQUEST_PRIMARY);
}

void Player::OnDisconnected()
{
    if (authorized(User::INGAME))
    {
        auto lock = Lock();
        SaveAllProperty();
        LeaveParty();
        World::Remove(this);
    }

    if (authorized(User::AUTHORIZED))
        Socket::DBClient().write(S2D_DISCONNECT, "d", GetAID());
}

void Player::OnStart(packet& p)
{
    auto unknown = p.pop<char>();
    auto height = p.pop<int>();

    OnCharacterAppear(*this, true);

    World::Add(this);

    assign(User::INGAME);
    deny(User::LOADING);
}

void Player::OnRestart(packet& p)
{
    if (p.pop<char>() == 1) // Can I logout?
        // 1=Yes, 0=No -> In Fight? PVP? Etc
        write(S2C_ANS_RESTART, "b", CanLogout() ? 1 : 0); // Yes, you can
    else
    {
        auto lock = Lock();
        SaveAllProperty();
        //m_inventory.Reset();
        m_inventory = Inventory();
        ResetStates();
        LeaveParty();
        //World::Map(GetMap()).Remove(this);
        World::Remove(this);
        deny(User::INGAME);
        Socket::DBClient().write(S2D_RESTART, "dd", GetUID(), GetAID());
    }
}

void Player::OnExit(packet& p)
{
    write(S2C_ANS_GAMEEXIT, "b", CanLogout() ? 1 : 0);
}

void Player::OnMove(packet& p, bool end)
{
    std::int8_t x, y, z;
    p >> x >> y >> z;
    World::Move(this, x, y, z, end);
}

void Player::OnLoadPlayer(packet& p)
{
    p >> m_data >> m_name >> m_x >> m_y;
    m_z = m_data.Z;
    m_map = m_data.Map;
    m_curhp = m_data.CurHP;
    m_curmp = m_data.CurMP;
}

void Player::OnLoadItems(packet& p)
{
    unsigned short count = p.pop<unsigned short>();

    for (unsigned short i = 0; i < count; i++)
    {
        auto info = p.pop<ITEMINFO>();
        m_inventory.Insert(info);
    }

    OnLoadFinish();
}

void Player::OnLoadFinish()
{
    write(S2C_PROPERTY, "bsbwwwwwwddwwwwwbIwwwwwwbbbbbd",
        0, //Grade
        "\0", //GuildName
        0, //GRole
        GetContribute(),
        GetBaseStrength(),
        GetBaseHealth(),
        GetBaseInteligence(),
        GetBaseWisdom(),
        GetBaseDexterity(),
        GetCurHP(),
        GetMaxHP(),
        GetCurMP(),
        GetMaxMP(),
        GetHit(),
        GetDodge(),
        GetDefense(),
        GetAbsorb(),
        GetExp(),
        GetMinAttack(),
        GetMaxAttack(),
        GetMinMagic(),
        GetMaxMagic(),
        GetPUPoint(),
        GetSUPoint(),
        GetResist(RT_FIRE),
        GetResist(RT_ICE),
        GetResist(RT_LITNING),
        GetResist(RT_CURSE),
        GetResist(RT_PALSY),
        GetRage());

    short time = 900;//1200;
    write(S2C_ANS_LOAD, "wdd", time, GetX(), GetY());
    //TODO: Change positions?

    write(m_inventory);

    // Send Inventory property
    SendInventoryProperty();
    
    spdlog::info("Player ID: {}; PID: {}; has loaded", GetID(), GetPID());
}

void Player::OnRest(packet& p)
{
    auto action = p.pop<char>();
    packet out(S2C_ACTION,"dbb", GetID(), AT_REST, action);

	if (action)
	{
		if (IsGState(CGS_REST))
            return;

        AddGState(CGS_REST);
        WriteInSight(out);
	}
	else
	{
		if (!IsGState(CGS_REST))
            return;

        SubGState(CGS_REST);
        WriteInSight(out);
	}
}

void Player::OnChatting(packet& p)
{
    auto message = p.pop_str();

    if (message.empty())
        return;

    packet message_packet(S2C_CHATTING);
    message_packet << GetName() << message;

    switch (message.at(0))
    {
        case '/':
        {
            CommandDispatcher::Dispatch(*this, message);
            break;
        }
        case '@':
        {
            auto space_pos = message.find(' ');

            if (space_pos == std::string::npos)
                return;

            std::string receiver_name = message.substr(1, space_pos - 1);

            if (receiver_name == GetName())
                return;

            if (!World::ForPlayerWithName(receiver_name, [&](Player& receiver) {
                write(message_packet);
                receiver.write(message_packet);
            }))
                write(S2C_MESSAGE, "d", MSG_THEREISNOPLAYER);

            break;
        }
        case '#':
        {
            if (message.size() <= 1)
                break;
            auto lock = Lock();
            if (HasParty())
                GetParty()->WriteToAll(message_packet);
            break;
        }
        default:
        {
            WriteInSight(message_packet);
            break;
        }
    }
}

void Player::OnPutOnItem(packet& p)
{
    auto local_id = p.pop<unsigned int>();
    auto item = m_inventory.FindByLocalID(local_id);
    if (!item)
        return;

    if (item->GetInit().LimitClass != PC_ALL && item->GetInit().LimitClass != GetClass())
        return;

    if (item->GetInit().LimitLevel > GetLevel())
        return;

    if (!m_inventory.PutOn(local_id))
        return;

    WriteInSight(packet(S2C_PUTONITEM, "ddw", 
        GetID(), 
        item->GetLocalID(), 
        item->GetInit().Index));

    Socket::DBClient().write(S2D_UPDATEITEMINFO, "dd", item->GetInfo().IID, item->GetInfo().Info);

    SendInventoryProperty();
}

void Player::OnPutOffItem(packet& p)
{
    auto local_id = p.pop<unsigned int>();

    auto item = m_inventory.PutOff(local_id);
    if (!item)
        return;

    WriteInSight(packet(S2C_PUTOFFITEM, "ddw", 
        GetID(), 
        item->GetLocalID(), 
        item->GetInit().Index));

    Socket::DBClient().write(S2D_UPDATEITEMINFO, "dd", item->GetInfo().IID, item->GetInfo().Info);

    SendInventoryProperty();
}

void Player::OnUseItem(packet& p)
{
    auto local_id = p.pop<unsigned int>();

    auto item = m_inventory.FindByLocalID(local_id);
    if (!item) return;

    if (item->GetInit().Class == IC_RIDE)
        WriteInSight(packet(S2C_RIDING, "bdd", 0, GetID(), 
            item->GetInit().RidingType)); // TODO: Change to buff.
}

void Player::OnTrashItem(bango::network::packet& p)
{
    TrashItem(p.pop<unsigned int>());
}

//! /get [index:int] [num:int]
void Player::OnGetItem(CommandDispatcher::Token& token)
{
    auto index = (int)token;
    auto num = (int)token;
    InsertItem(index, num <= 0 ? 1 : num);
}

void Player::OnCharacterAppear(Character& subject, bool hero)
{
    write(subject.BuildAppearPacket(hero));
}

void Player::OnCharacterDisappear(Character& subject)
{
    write(subject.BuildDisappearPacket());
}

void Player::OnCharacterMove(Character& subject, std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop)
{
    write(subject.BuildMovePacket(delta_x, delta_y, delta_z, stop));
}

void Player::InsertItem(unsigned short index, unsigned int num)
{
    auto item = m_inventory.FindByIndex(index);
    if (item && item->GetInit().Plural)
    {
        item->UpdateNum(item->GetInfo().Num+num);
        write(S2C_UPDATEITEMNUM, "ddb", item->GetLocalID(), item->GetInfo().Num, TL_CREATE);
        Socket::DBClient().write(S2D_UPDATEITEMNUM, "dd", item->GetInfo().IID, item->GetInfo().Num);
    }
    else
    {
        ITEMINFO info = {};
        info.Index = index;
        try {
            const auto& init = InitItem::DB().at(index);
            info.CurEnd = init->Endurance;
            info.Num = init->Plural ? num : 1;
        } catch (const std::exception&) { return; }
        item = m_inventory.Insert(info);
        write(((packet)*item).change_type(S2C_INSERTITEM));
        //db insert
        packet out(S2D_INSERTITEM);
        out << item->GetInfo() << GetPID() << GetUID() << item->GetLocalID();
        Socket::DBClient().write(out);
    }
}

bool Player::TrashItem(unsigned int local)
{
    auto iid = m_inventory.GetIID(local);

    if (!m_inventory.Trash(local))
        return false;

    write(S2C_UPDATEITEMNUM, "ddb", local, 0, TL_DELETE);
    Socket::DBClient().write(S2D_TRASHITEM, "d", iid);

    return true;
}

void Player::Teleport(int x, int y, int z)
{
    char map=0, cheat=0;

    m_teleport_x = x;
    m_teleport_y = y;

    write(S2C_TELEPORT, "bdddb", map, x, y, z, cheat);
}

void Player::OnTeleportAnswer(packet& p)
{
    // TODO: Add packet hack checks or force teleportation after certain amount of time.
    if (m_teleport_x == 0)
        return;

    auto answer = p.pop<char>();
    auto z = p.pop<int>();

    if (answer)
        World::Teleport(this, m_teleport_x, m_teleport_y, z);

    m_teleport_x = m_teleport_y = 0;
}

void Player::OnMoveTo(CommandDispatcher::Token& token)
{
    int x = token;
    int y = token;

    Teleport(x, y);
}

packet Player::BuildAppearPacket(bool hero) const
{
    packet p(S2C_CREATEPLAYER);

    p   << GetID()
        << GetName()
        << GetClass(hero) //TODO: Remove hero from param.
        << GetX() 
        << GetY() 
        << GetZ() 
        << GetDir() 
        << GetGState()
        //<< GetEquipment()
        << m_inventory.GetEquipment()
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

packet Player::BuildDisappearPacket() const
{
    return packet(S2C_REMOVEPLAYER, "d", GetID());
}

packet Player::BuildMovePacket(std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop) const
{
    return packet(stop ? S2C_MOVEPLAYER_END : S2C_MOVEPLAYER_ON, "dbbb", GetID(), delta_x, delta_y, delta_z);
}

std::uint32_t Player::GetMaxHP() const
{
	return (
        (GetLevel() >= 96 ? 195 :
		(GetLevel() >= 91 ? 141.8147 :
		(GetLevel() >= 86 ? 111.426 :
		(GetLevel() >= 81 ? 91.758 :
		(GetLevel() >= 76 ? 78 :
		(GetLevel() >= 72 ? 67.8162 :
                            52)))))) * GetLevel() / 3) + 115 + 2 * GetHealth() * GetHealth() / g_denoHP[GetClass()] + m_inventory.GetHP(); //+ m_dwMaxHPAdd;
}

std::uint32_t Player::GetMaxMP() const
{
	return (
        (GetLevel() >= 96 ? 20 :
		(GetLevel() >= 91 ? 18 :
		(GetLevel() >= 86 ? 16 :
		(GetLevel() >= 81 ? 14 :
		(GetLevel() >= 76 ? 12 :
		(GetLevel() >= 72 ? 10 :
                            8)))))) * GetLevel()) + 140 + GetWisdom() + 2 * GetWisdom() * GetWisdom() / g_denoMP[GetClass()] + m_inventory.GetMP();// + m_wMaxMPAdd;
}

// std::uint16_t Player::GetResist(std::uint8_t type) const
// {
// 	switch (type)
// 	{
//         case RT_FIRE:
//             return GetInteligence() / 9 + m_inventory.GetResist(type);
//         case RT_ICE:
//             return GetInteligence() / 9 + m_inventory.GetResist(type);
//         case RT_LITNING:
//             return GetInteligence() / 9 + m_inventory.GetResist(type);
//         case RT_PALSY:
//             return GetHealth()      / 9 + m_inventory.GetResist(type);
//         case RT_CURSE:
//             return GetWisdom()      / 9 + m_inventory.GetResist(type);
//     }
// }

void Player::SendInventoryProperty() const
{
    SendProperty(P_STRADD);
    SendProperty(P_HTHADD);
    SendProperty(P_INTADD);
    SendProperty(P_WISADD);
    SendProperty(P_DEXADD);

    SendProperty(P_ABSORB);
    SendProperty(P_DEFENSE);
    SendProperty(P_ASPEED);
}

void Player::SendProperty(std::uint8_t kind, std::int64_t amount) const
{
    switch (kind)
    {
        case P_STR:
            write(S2C_UPDATEPROPERTY, "bwwww",  P_STR, GetBaseStrength(), GetHit(), GetMinAttack(), GetMaxAttack()); break;
        case P_HTH:
            write(S2C_UPDATEPROPERTY, "bwddw",  P_HTH, GetBaseHealth(), GetCurHP(), GetMaxHP(), GetResist(RT_PALSY)); break;
        case P_INT:
            write(S2C_UPDATEPROPERTY, "bwwwwww",P_INT, GetBaseInteligence(), GetMinMagic(), GetMaxMagic(), GetResist(RT_FIRE), GetResist(RT_ICE), GetResist(RT_LITNING)); break;
        case P_WIS:
            write(S2C_UPDATEPROPERTY, "bwwwwww",P_WIS, GetBaseWisdom(), GetCurMP(), GetMaxMP(), GetMinMagic(), GetMaxMagic(), GetResist(RT_CURSE)); break;
        case P_DEX:
            write(S2C_UPDATEPROPERTY, "bwwwwww",P_DEX, GetBaseDexterity(), GetHit(), GetDodge(), GetDodge(), GetMinAttack(), GetMaxAttack()); break;
        case P_STRADD:
            write(S2C_UPDATEPROPERTY, "bwwww",  P_STRADD, GetStrength()-GetBaseStrength(), GetHit(), GetMinAttack(), GetMaxAttack()); break;
        case P_HTHADD:
            write(S2C_UPDATEPROPERTY, "bwddw",  P_HTHADD, GetHealth()-GetBaseHealth(), GetCurHP(), GetMaxHP(), GetResist(RT_PALSY)); break;
        case P_INTADD:
            write(S2C_UPDATEPROPERTY, "bwwwwww",P_INTADD, GetInteligence()-GetBaseInteligence(), GetMinMagic(), GetMaxMagic(), GetResist(RT_FIRE), GetResist(RT_ICE), GetResist(RT_LITNING)); break;
        case P_WISADD:
            write(S2C_UPDATEPROPERTY, "bwwwwww",P_WISADD, GetWisdom()-GetBaseWisdom(), GetCurMP(), GetMaxMP(), GetMinMagic(), GetMaxMagic(), GetResist(RT_CURSE)); break;
        case P_DEXADD:
            write(S2C_UPDATEPROPERTY, "bwwwwww",P_DEXADD, GetDexterity()-GetBaseDexterity(), GetHit(), GetDodge(), GetDodge(), GetMinAttack(), GetMaxAttack()); break;
        case P_ABSORB:
            write(S2C_UPDATEPROPERTY, "bw",     P_ABSORB, GetAbsorb()); break;
        case P_DEFENSE:
            write(S2C_UPDATEPROPERTY, "bww",    P_DEFENSE, GetDefense(), GetDefense()); break;
        case P_DODGE:
            write(S2C_UPDATEPROPERTY, "bww",    P_DODGE, GetDodge(), GetDodge()); break;
        case P_PUPOINT:
            write(S2C_UPDATEPROPERTY, "bw",     P_PUPOINT, GetPUPoint()); break;
        case P_SUPOINT:
            write(S2C_UPDATEPROPERTY, "bw",     P_SUPOINT, GetSUPoint()); break;
        case P_LEVEL:
            write(S2C_UPDATEPROPERTY, "bw",     P_LEVEL, GetLevel()); break;
        case P_ASPEED:
            write(S2C_UPDATEPROPERTY, "bw",     P_ASPEED, GetAttackSpeed()); break; //TODO: Should be updated on buff aswell
        case P_EXP:
            write(S2C_UPDATEPROPERTY, "bII",    P_EXP, GetExp(), amount); break;
    }
}

void Player::SaveAllProperty() const
{
    packet p(S2D_SAVEALLPROPERTY);
    p   << GetPID()
        << GetLevel()
        << GetX()
        << GetY()
        << GetZ()
        << GetContribute()
        << GetCurHP()
        << GetCurMP()
        << GetExp()
        << GetPUPoint()
        << GetSUPoint()
        << GetRage();
    Socket::DBClient().write(p);
}
std::uint16_t Player::GetReqPU(std::uint8_t* stats)
{
	std::uint16_t req=0;

	if (GetClass() == PC_KNIGHT)
		req += FIND_NEED_PU_EX(GetBaseStrength(), stats[P_STR]);
	else
		req += FIND_NEED_PU(GetBaseStrength(), stats[P_STR]);

	if (GetClass() == PC_MAGE || GetClass() == PC_SHAMAN)
		req += FIND_NEED_PU_EX(GetBaseInteligence(), stats[P_INT]);
	else
		req += FIND_NEED_PU(GetBaseInteligence(), stats[P_INT]);

	if (GetClass() == PC_ARCHER || GetClass() == PC_THIEF)
		req += FIND_NEED_PU_EX(GetBaseDexterity(), stats[P_DEX]);
	else
		req += FIND_NEED_PU(GetBaseDexterity(), stats[P_DEX]);

	req += FIND_NEED_PU(GetBaseHealth(), stats[P_HTH]);
	req += FIND_NEED_PU(GetBaseWisdom(), stats[P_WIS]);

    return req;
}

void Player::OnUpdateProperty(packet& p)
{
    std::uint8_t stats[5] = {
        p.pop<std::uint8_t>(),
        p.pop<std::uint8_t>(),
        p.pop<std::uint8_t>(),
        p.pop<std::uint8_t>(),
        p.pop<std::uint8_t>()
    };

    if (stats[P_STR]+GetBaseStrength()      > MAX_STAT_DISTRIBUTED) return;
    if (stats[P_HTH]+GetBaseHealth()        > MAX_STAT_DISTRIBUTED) return;
    if (stats[P_INT]+GetBaseInteligence()   > MAX_STAT_DISTRIBUTED) return;
    if (stats[P_WIS]+GetBaseWisdom()        > MAX_STAT_DISTRIBUTED) return;
    if (stats[P_DEX]+GetBaseDexterity()     > MAX_STAT_DISTRIBUTED) return;

    auto required = GetReqPU(stats);

    if (GetPUPoint() < required)
        return;

    m_data.Strength     += stats[P_STR];
    m_data.Health       += stats[P_HTH];
    m_data.Inteligence  += stats[P_INT];
    m_data.Wisdom       += stats[P_WIS];
    m_data.Dexterity    += stats[P_DEX];
    m_data.PUPoint      -= required;

    SendProperty(P_STR);
    SendProperty(P_HTH);
    SendProperty(P_INT);
    SendProperty(P_WIS);
    SendProperty(P_DEX);
    SendProperty(P_PUPOINT);

    Socket::DBClient().write(S2D_UPDATEPROPERTY, "dwwwwww", 
        GetPID(), 
        GetBaseStrength(), 
        GetBaseHealth(), 
        GetBaseInteligence(), 
        GetBaseWisdom(), 
        GetBaseDexterity(), 
        GetPUPoint());
}

void Player::OnPlayerAnimation(packet& p)
{
    auto id = p.pop<Character::id_t>();
    auto animation = p.pop<unsigned char>();

    if (animation < 20)
        WriteInSight(packet(S2C_PLAYER_ANIMATION, "db", GetID(), animation));
}

void Player::OnAttack(packet& p)
{
    auto kind = p.pop<char>();
    auto id = p.pop<Character::id_t>();
    auto z = p.pop<unsigned int>();

    World::Map(GetMap()).For(WorldMap::QK_PLAYER | WorldMap::QK_MONSTER, id, [&](Character& character) 
    {
        // Check if is both actors are valid
        // Check for GState 4?+
        // Range check
        // CanAttack
        // OnPVP

        auto lock = Lock();

        if (!m_inventory.HasWeapon())
            return;
        
        auto now = time::now();

        auto duration = (now-m_last_attack).count();
        auto damage_reduce = (double) duration / (double) GetAttackSpeed();

        if (damage_reduce > 1.f)
            damage_reduce = 1.f;

        else if (damage_reduce < 0.6f)
            return;

        m_last_attack=now;

        LookAt(&character);

        auto defender_lock = character.Lock();

        if(character.IsGState(CGS_KNEE) || character.IsGState(CGS_KO))
            return;

        // CheckBlock
        if (!CheckHit(&character))
        {
            WriteInSight(packet(S2C_ATTACK, "ddddb",
                GetID(), character.GetID(), 0, 0, ATF_MISS));
            return;
        }

        std::int64_t damage = GetAttack();
        damage *= damage_reduce;
        // Something with mage
        damage = character.GetFinalDamage(this, damage);
        // Apply Mix Effects
        lock.unlock();

        if (damage < 0)
            return;
        if ((uint64_t)damage > character.GetCurHP())
            damage = character.GetCurHP();

        WriteInSight(packet(S2C_ATTACK, "ddddb", 
            GetID(), 
            character.GetID(), 
            damage,
            0,//EB
            damage == 0 ? ATF_IGNORE : ATF_HIT));

        character.ReceiveDamage(GetID(), damage);

        if (character.GetCurHP() <= 0)
            character.Die();
    });
}

void Player::OnAskParty(packet& p)
{
    auto invited_player_id = p.pop<id_t>();
    if (invited_player_id == GetID())
        return;

    auto lock = Lock();
    if (HasParty() && GetParty()->GetLeader() != this)
    {
        write(S2C_MESSAGE, "b", MSG_NORIGHTOFPARTYHEAD);
        return;
    }

    World::ForPlayer(invited_player_id, [&](Player& invited_player){
        if (distance(&invited_player) > MAP_SIGHT)
            return;

        auto invited_player_lock = invited_player.Lock();
        if (invited_player.HasParty())
        {
            // TODO: Update bango::network::packet to accept std::string as well (remove the need of c_str() call)
            write(S2C_MESSAGEV, "bs", MSG_JOINEDINOTHERPARTY, invited_player.GetName().c_str());
            return;
        }

        write(S2C_MESSAGEV, "bs", MSG_ASKJOINPARTY, invited_player.GetName().c_str());
        invited_player.SetPartyInviterID(GetID());
        invited_player.write(S2C_ASKPARTY, "d", GetID());
    });
}

void Player::OnAskPartyAnswer(packet& p)
{
    auto answer = p.pop<bool>();
    auto inviter_id = p.pop<id_t>();

    if (inviter_id == GetID())
        return;

    auto lock = Lock();

    if (HasParty())
        return;

    if (inviter_id != GetPartyInviterID())
        return;

    World::ForPlayer(inviter_id, [&](Player& inviter) {
        ResetPartyInviterID();

        if (distance(&inviter) > MAP_SIGHT)
            return;

        if (answer == false)
        {
            inviter.write(S2C_MESSAGEV, "bs", MSG_REJECTJOINPARTY, GetName().c_str());
            return;
        }

        auto inviter_lock = inviter.Lock();
        if (!inviter.HasParty())
        {
            auto party = std::make_shared<Party>(&inviter, this);
            inviter.SetParty(party);
            SetParty(party);
            World::AddParty(party);
        }
        else if (inviter.IsPartyLeader())
        {
            if (inviter.GetParty()->AddMember(this))
                SetParty(inviter.GetParty());
        }
    });
}

void Player::OnLeaveParty(packet& p)
{
    auto lock = Lock();
    LeaveParty();
}

void Player::OnItemPick(packet& p)
{
    auto item_id = p.pop<id_t>();
    auto x = p.pop<std::int32_t>();
    auto y = p.pop<std::int32_t>();

    World::ForLoot(item_id, [&](Loot& loot) {
                    auto& info = loot.GetItemInfo();
                    InsertItem(info.Index, info.Num);
                    World::RemoveLootById(item_id);
                });
}

void Player::LeaveParty(bool is_kicked)
{
    if (HasParty())
    {
        GetParty()->RemoveMember(this, is_kicked);
        if (GetParty()->GetSize() == 1)
        {
            auto* leader = GetParty()->GetLeader();
            auto leader_lock = leader->Lock();
            if (leader->HasParty())
            {
                World::RemoveParty(leader->GetParty());
                leader->GetParty()->RemoveMember(leader);
                leader->ResetParty();
            }
        }
        ResetParty();
    }
}

void Player::OnExileParty(bango::network::packet& p)
{
    auto banned_player_id = p.pop<int>();
    BanFromParty(banned_player_id);
}

void Player::BanFromParty(id_t banned_player_id)
{
    if (banned_player_id == GetID())
        return;

    auto lock = Lock();
    if (!IsPartyLeader())
        return;

    World::ForPlayer(banned_player_id, [&](Player& banned_player) {
        auto banned_player_lock = banned_player.Lock();
        if (!banned_player.HasParty())
            return;
        if (banned_player.GetParty() != GetParty())
            return;
        GetParty()->RemoveMember(&banned_player);
        banned_player.ResetParty();
        if (GetParty()->GetSize() == 1)
        {
            GetParty()->RemoveMember(this);
            ResetParty();
        }
    });
}

void Player::Tick()
{
}

void Player::Die()
{
    AddGState(CGS_KO);
    WriteInSight(bango::network::packet(S2C_ACTION, "db", GetID(), AT_DIE));
}

void Player::UpdateExp(std::int64_t amount)
{
    // No effect
    if (amount == 0)
        return;
    
    // Decrease
    if (amount < 0)
    {
        if (std::cmp_greater(std::abs(amount), m_data.Exp))
            amount = -static_cast<std::int64_t>(m_data.Exp);
        m_data.Exp += amount;
        SendProperty(P_EXP, amount);
        return;
    }

    if (GetLevel() >= MAX_LEVEL)
    {
        spdlog::warn("Level is too large to recieve exp: {}", GetLevel());
        return;   
    }

    // Increase
    m_data.Exp += amount;  // TODO: Check for possible overflow

    std::uint64_t required_exp = g_n64NeedExpFinal[GetLevel()];
    while (m_data.Exp > required_exp)
    {
        spdlog::debug("More exp than required ({}/{}). Performing level up from {} to {}.", m_data.Exp, required_exp,
            GetLevel(), GetLevel()+1);
        m_data.Exp -= required_exp;
        LevelUp();
        required_exp = g_n64NeedExpFinal[GetLevel()];
    }

    SendProperty(P_EXP, amount);
    return;
}

bool Player::CanReciveExp()
{
    return !IsGState(CGS_KNEE | CGS_KO | CGS_FISH);
}

std::uint64_t Player::CalculateExp(std::uint64_t exp, std::uint8_t monster_level)
{
    //TODO: Calculate exp buffs like exp stone, asadal, exp event.
    std::int32_t level_difference =  static_cast<std::int32_t>(monster_level - GetLevel());
    if (level_difference < 0)
    {
        level_difference = std::min(std::abs(level_difference), 20);
        //When monster_level < player_level, use g_nReviseExpB to calculate color ratio
        exp = (exp - exp * (g_nReviseExpB[(GetLevel() - 1) / 10][level_difference] / 100.0));
        return exp;
    }

    level_difference = std::min(level_difference, 20);
    //When monster_level > player_level, use g_nReviseExpA to calculate color ratio
    exp = (exp * (g_nReviseExpA[(GetLevel() - 1) / 10][level_difference] / 100.0) + exp);
    return exp;
}

void Player::LevelUp()
{
    m_data.SUPoint += 1;
    m_data.Level += 1;
    m_data.PUPoint += GET_PU_ON_LEVEL_UP(GetLevel());
    SendProperty(P_LEVEL);
    SendProperty(P_SUPOINT);
    SendProperty(P_PUPOINT);
    ApplyVisualEffect(E_LEVELUP);
    //m_data.Exp = 0;
}
