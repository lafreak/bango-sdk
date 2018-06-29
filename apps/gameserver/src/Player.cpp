#include "Player.h"

#include "Socket.h"
#include "World.h"

using namespace bango::network;

void Player::OnConnected()
{
    assign(User::CAN_REQUEST_PRIMARY);
}

void Player::OnDisconnected()
{
    if (authorized(User::INGAME))
        World::Remove(this);

    if (authorized(User::AUTHORIZED))
        Socket::DBClient().write(S2D_DISCONNECT, "d", GetAID());
}

void Player::OnStart(packet& p)
{
    auto unknown = p.pop<char>();
    auto height = p.pop<int>();

    //write(BuildAppearPacket(true));
    OnCharacterAppear(this, true);

    World::Add(this);

    assign(User::INGAME);
    deny(User::LOADING);
}

void Player::OnRestart(packet& p)
{
    if (p.pop<char>() == 1) // Can I logout?
        // 1=Yes, 0=No -> In Fight? PVP? Etc
        write(S2C_ANS_RESTART, "b", CanLogout() ? 1 : 0); 
    else
    {
        Inventory::Reset();
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
        Inventory::Insert(info);
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
        GetMaxHP(),//GetCurHP(), //MaxHP
        GetCurMP(),
        GetMaxMP(),//GetCurMP(), //MaxMP
        GetHit(),//1, //Hit
        GetDodge(),//2, //Dodge
        GetDefense(),//3, //Defense
        GetAbsorb(),//4, //Absorb
        GetExp(),
        GetMinAttack(),//5, //MinAttack
        GetMaxAttack(),//6, //MaxAttack
        GetMinMagic(),//7, //MinMagic
        GetMaxMagic(),//8, //MaxMagic
        GetPUPoint(),
        GetSUPoint(),
        GetResist(RT_FIRE),//9, //ResistFire
        GetResist(RT_ICE),//10, //ResistIce
        GetResist(RT_LITNING),//11, //ResistLitning
        GetResist(RT_CURSE),//12, //ResistCurse
        GetResist(RT_PALSY),//13, //ResistPalsy
        GetRage());

    short time = 1200;
    write(S2C_ANS_LOAD, "wdd", time, GetX(), GetY());
    //TODO: Change positions?

    write((Inventory)*this);

    // Send Inventory property
    SendInventoryProperty();
}

void Player::OnRest(packet& p)
{
    auto action = p.pop<char>();
    std::cout << "On Rest " << (int)action << std::endl;
}

void Player::OnChatting(packet& p)
{
    auto message = p.pop_str(); // BUG: Empty packet will throw an exception.

    // if (message[0] == '/') // BUG: Message might be empty.
    if (message[0] == '/')
        return CommandDispatcher::Dispatch(this, message);

    packet out(S2C_CHATTING);
    out << GetName() << message;

    World::Map(GetMap()).WriteInSight(this, out);
}

void Player::OnPutOnItem(packet& p)
{
    auto local_id = p.pop<unsigned int>();
    auto item = FindByLocalID(local_id);
    if (!item)
        return;

    if (item->GetInit().LimitClass != PC_ALL && item->GetInit().LimitClass != GetClass())
        return;

    if (item->GetInit().LimitLevel > GetLevel())
        return;

    if (!Inventory::PutOn(local_id))
        return;

    World::Map(GetMap()).WriteInSight(this, 
        packet(S2C_PUTONITEM, "ddw", 
            GetID(), 
            item->GetLocalID(), 
            item->GetInit().Index));

    Socket::DBClient().write(S2D_UPDATEITEMINFO, "dd", item->GetInfo().IID, item->GetInfo().Info);

    SendInventoryProperty();
}

void Player::OnPutOffItem(packet& p)
{
    auto local_id = p.pop<unsigned int>();

    auto item = Inventory::PutOff(local_id);
    if (!item)
        return;

    World::Map(GetMap()).WriteInSight(this,
        packet(S2C_PUTOFFITEM, "ddw", 
            GetID(), 
            item->GetLocalID(), 
            item->GetInit().Index));

    Socket::DBClient().write(S2D_UPDATEITEMINFO, "dd", item->GetInfo().IID, item->GetInfo().Info);

    SendInventoryProperty();
}

void Player::OnUseItem(packet& p)
{
    auto local_id = p.pop<unsigned int>();

    auto item = Inventory::FindByLocalID(local_id);
    if (!item) return;

    if (item->GetInit().Class == IC_RIDE)
        World::Map(GetMap()).WriteInSight(this, 
            packet(S2C_RIDING, "bdd", 
                0, 
                GetID(), 
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

void Player::OnCharacterAppear(Character * subject, bool hero)
{
    write(subject->BuildAppearPacket(hero));
}

void Player::OnCharacterDisappear(Character * subject)
{
    write(subject->BuildDisappearPacket());
}

void Player::OnCharacterMove(Character * subject, std::int8_t delta_x, std::int8_t delta_y, std::int8_t delta_z, bool stop)
{
    write(subject->BuildMovePacket(delta_x, delta_y, delta_z, stop));
}

void Player::InsertItem(unsigned short index, unsigned int num)
{
    auto item = Inventory::FindByIndex(index);
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
        auto item = Inventory::Insert(info);
        write(((packet)*item).change_type(S2C_INSERTITEM));
        //db insert
        packet out(S2D_INSERTITEM);
        out << item->GetInfo() << GetPID() << GetUID() << item->GetLocalID();
        Socket::DBClient().write(out);
    }
}

bool Player::TrashItem(unsigned int local)
{
    auto iid = Inventory::GetIID(local);

    if (!Inventory::Trash(local))
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
    //TODO: Add buffs.
	return (
        (GetLevel() >= 96 ? 195 :
		(GetLevel() >= 91 ? 141.8147 :
		(GetLevel() >= 86 ? 111.426 :
		(GetLevel() >= 81 ? 91.758 :
		(GetLevel() >= 76 ? 78 :
		(GetLevel() >= 72 ? 67.8162 :
                            52)))))) * GetLevel() / 3) + 115 + 2 * GetHealth() * GetHealth() / g_denoHP[GetClass()] + Inventory::GetAddHP(); //+ m_dwMaxHPAdd;
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
                            8)))))) * GetLevel()) + 140 + GetWisdom() + 2 * GetWisdom() * GetWisdom() / g_denoMP[GetClass()] + Inventory::GetAddMP();// + m_wMaxMPAdd;
}

void Player::SendInventoryProperty()
{
    //TODO: Total-Base?
    write(S2C_UPDATEPROPERTY, "bwwww",   P_STRADD, Inventory::GetAddStrength(), GetHit(), GetMinAttack(), GetMaxAttack());
    write(S2C_UPDATEPROPERTY, "bwddw",   P_HTHADD, Inventory::GetAddHealth(), GetCurHP(), GetMaxHP(), GetResist(RT_PALSY));
    write(S2C_UPDATEPROPERTY, "bwwwwww", P_INTADD, Inventory::GetAddInteligence(), GetMinMagic(), GetMaxMagic(), GetResist(RT_FIRE), GetResist(RT_ICE), GetResist(RT_LITNING));
    write(S2C_UPDATEPROPERTY, "bwwwwww", P_WISADD, Inventory::GetAddWisdom(), GetCurMP(), GetMaxMP(), GetMinMagic(), GetMaxMagic(), GetResist(RT_CURSE));
    write(S2C_UPDATEPROPERTY, "bwwwwww", P_DEXADD, Inventory::GetAddDexterity(), GetHit(), GetDodge(), GetDodge(), GetMinAttack(), GetMaxAttack());

    write(S2C_UPDATEPROPERTY, "bw", P_ABSORB, GetAbsorb());
    write(S2C_UPDATEPROPERTY, "bww", P_DEFENSE, GetDefense(), GetDefense());
}

void Player::Tick()
{
}