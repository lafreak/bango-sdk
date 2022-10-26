#include "Inventory.h"

#include "spdlog/spdlog.h"

#include <cassert>
#include <algorithm>

#include <bango/processor/db.h>
#include <bango/network/packet.h>

using namespace bango::processor;
using namespace bango::network;

void InitItem::set(lisp::var param)
{
    switch (FindAttribute(param.pop()))
    {
        case A_INDEX:           Index       = param.pop(); break;
        case A_LEVEL:           Level       = param.pop(); break;
        case A_ENDURANCE:       Endurance   = param.pop(); break;
        case A_WEARABLE:        Wearable    = param.pop(); break;
        case A_PLURAL:          Plural      = param.pop(); break;
        case A_RIDINGTYPE:      RidingType  = param.pop(); break;
        case A_CLASS:           Class       = FindAttribute(param.pop());
                                Kind        = FindAttribute(param.pop());
                                break;
        case A_LIMIT:           LimitClass  = FindAttribute(param.pop());
                                LimitLevel  = param.pop();
                                break;
        case A_SPECIALTY:
        {
            while (param.consp())
            {
                lisp::var p = param.pop();
                switch (FindAttribute(p.pop()))
                {
                    case A_STR: Specialty.Stats[P_STR] = p.pop(); break;
                    case A_HTH: Specialty.Stats[P_HTH] = p.pop(); break;
                    case A_WIS: Specialty.Stats[P_WIS] = p.pop(); break;
                    case A_INT: Specialty.Stats[P_INT] = p.pop(); break;
                    case A_DEX: Specialty.Stats[P_DEX] = p.pop(); break;

                    case A_HP:  Specialty.HP =            p.pop(); break;
                    case A_MP:  Specialty.MP =            p.pop(); break;
                    case IC_DEFENSE:Specialty.Defense =   p.pop(); break;
                    case A_HIT:     Specialty.Hit =       p.pop(); break;
                    case A_DODGE:   Specialty.Dodge =     p.pop(); break;
                    case A_ABSORB:  Specialty.Absorb =    p.pop(); break;
                    case A_ASPEED:  Specialty.AttackSpeed=p.pop(); break;

                    case A_ATTACK:      Specialty.MinAttack = p.pop();
                                        Specialty.MaxAttack = p.pop();
                                        break;

                    case A_MAGICATTACK: Specialty.MinMagic = p.pop();
                                        Specialty.MaxMagic = p.pop();
                                        break;

                    case A_RESISTCURSE:     Specialty.Resists[RT_CURSE]     = p.pop(); break;
                    case A_RESISTFIRE:      Specialty.Resists[RT_FIRE]      = p.pop(); break;
                    case A_RESISTICE:       Specialty.Resists[RT_ICE]       = p.pop(); break;
                    case A_RESISTLITNING:   Specialty.Resists[RT_LITNING]   = p.pop(); break;
                    case A_RESISTPALSY:     Specialty.Resists[RT_PALSY]     = p.pop(); break;
                }
            }
        }
        break;
    }

    WearId = FindWearId(Kind);
}

Item::Item(const InitItem* init, const ITEMINFO& info)
    : m_init(init), m_info(info)
{
    static unsigned int g_max_local_id=0;
    m_local_id = ++g_max_local_id;
}

Item::operator packet() const
{
    packet p;

    p   << m_info.Index 
        << GetLocalID() 
        << m_info.Prefix
        << m_info.Info
        << m_info.Num
        << m_init->Endurance
        << m_info.CurEnd
        << m_info.SetGem
        << m_info.XAttack
        << m_info.XMagic
        << m_info.XDefense
        << m_info.XHit
        << m_info.XDodge
        << m_info.ProtectNum
        << m_info.WeaponLevel
        << m_info.CorrectionAddNum
        << m_info.Unknown1
        << m_info.RemainingSeconds
        << m_info.RemainingMinutes
        << m_info.RemainingHours
        << m_info.FuseInfo
        << m_info.Shot
        << m_info.Perforation
        << m_info.GongA
        << m_info.GongB;

    return p;
}

void Inventory::AddVisibleIndex(unsigned short index)
{
    for (int i = 0; i < EQUIPMENT_SIZE; i++) 
    {
        if (!m_equipment.Index[i]) 
        {
            m_equipment.Index[i] = index;
            break;
        }
    }
}

void Inventory::RemoveVisibleIndex(unsigned short index)
{
    for (int i = 0; i < EQUIPMENT_SIZE; i++) 
    {
        if (m_equipment.Index[i] == index)
            m_equipment.Index[i] = 0;
    }
}

const Item::Ptr Inventory::Insert(const ITEMINFO& info)
{
    try {
        auto& init = InitItem::DB().at(info.Index);

        auto item = std::make_shared<Item>(init.get(), info);
        auto result = m_items.insert(std::make_pair(item->GetLocalID(), item));
        if (! result.second)
            throw std::runtime_error("Duplicate item local ID");

        // TODO: Temporary
        if (init->Wearable && info.Info & ITEM_PUTON) 
        {
            assert(init->WearId >= 0); // BUG: May trigger when items were put on manually trough DB.
            m_wear_items[init->WearId] = item;
            AddVisibleIndex(info.Index);
            ApplySpecialty(item);
        }

        if (init->Class == IC_RIDE && info.Info & ITEM_PUTON)
        {
            m_ride = item;
            AddVisibleIndex(info.Index);
        }

        return item;

    } catch (const std::out_of_range&) {
        spdlog::error("Tried to create item not existing in InitItem; index: {}", info.Index);
    } catch (const std::runtime_error& e) {
        spdlog::error("Cannot create item with error: {}", e.what());
    }

    return nullptr;
}

void Inventory::Reset()
{
   // m_items.clear();
    //m_ride = nullptr;

    // m_hp=0, m_mp=0, m_def=0, m_hit=0, m_dodge=0, m_absorb=0;
    // m_aspeed=0, m_minattack=0, m_maxattack=0, m_minmagic=0, m_maxmagic=0;

    m_stats[P_HTH]=0;
    m_stats[P_STR]=0;
    m_stats[P_WIS]=0;
    m_stats[P_INT]=0;
    m_stats[P_DEX]=0;

    m_resists[RT_CURSE]=0;
    m_resists[RT_FIRE]=0;
    m_resists[RT_ICE]=0;
    m_resists[RT_LITNING]=0;
    m_resists[RT_PALSY]=0;

    for (int i = 0; i < EQUIPMENT_SIZE; i++)
        m_equipment.Index[i] = 0;

    for (int i = 0; i < WS_LAST; i++)
        m_wear_items[i] = nullptr;
}

const Item::Ptr Inventory::FindByIndex(unsigned short index) const
{
    for (const auto& p : m_items)
        if (p.second->GetInfo().Index == index)
            return p.second;
    return nullptr;
}

const Item::Ptr Inventory::FindByLocalID(const std::uint32_t local) const
{
    try {
        return m_items.at(local);
    } catch (const std::exception&) {
        return nullptr;
    }
}

bool Inventory::Trash(const std::uint32_t local)
{
    auto item = FindByLocalID(local);
    if (!item)
        return false;

    if (item->GetInfo().Info & ITEM_PUTON)
        return false;

    m_items.erase(local);
    return true;
}

const Item::Ptr Inventory::PutOn(const std::uint32_t local)
{
    auto item = FindByLocalID(local);
    if (!item)
        return nullptr;

    if (item->GetInfo().Info & ITEM_PUTON)
        return nullptr;

    if (item->GetInit().Class == IC_RIDE)
    {
        if (m_ride)
            return nullptr;
        
        m_ride = item;
        item->AddInfo(ITEM_PUTON);
        AddVisibleIndex(item->GetInit().Index);
        return item;
    }

    if (!item->GetInit().Wearable)
        return nullptr;

    if (item->GetInit().WearId == -1)
        return nullptr;

    if (item->GetInit().Kind == ISC_SWORD2HAND)
        if (m_wear_items[WS_SHIELD])
            return nullptr;

    if (item->GetInit().Kind == ISC_SHIELD)
        if (m_wear_items[WS_WEAPON])
            if (m_wear_items[WS_WEAPON]->GetInit().Kind == ISC_SWORD2HAND)
                return nullptr;

    if (m_wear_items[item->GetInit().WearId] != nullptr)
        return nullptr;

    m_wear_items[item->GetInit().WearId] = item;
    item->AddInfo(ITEM_PUTON);

    AddVisibleIndex(item->GetInit().Index);
    ApplySpecialty(item);

    return item;
}

const Item::Ptr Inventory::PutOff(const std::uint32_t local)
{
    auto item = FindByLocalID(local);
    if (!item)
        return nullptr;

    if (! (item->GetInfo().Info & ITEM_PUTON) )
        return nullptr;

    if (item->GetInit().Class == IC_RIDE)
    {
        if (!m_ride)
            return nullptr;
        
        m_ride = nullptr;
        item->SubInfo(ITEM_PUTON);
        RemoveVisibleIndex(item->GetInit().Index);
        return item;
    }

    assert(item->GetInit().Wearable);
    assert(item->GetInit().WearId != -1);
    assert(m_wear_items[item->GetInit().WearId] != nullptr);

    m_wear_items[item->GetInit().WearId] = nullptr;
    item->SubInfo(ITEM_PUTON);

    RemoveVisibleIndex(item->GetInit().Index);
    FreeSpecialty(item);

    return item;
}

Inventory::operator packet() const
{
    packet p(S2C_ITEMINFO);
    p << (unsigned short) m_items.size();

    for (const auto& pair : m_items)
        p << (packet) *(pair.second.get());

    return p;
}

void Inventory::UpdateItemIID(const std::uint32_t local, int iid)
{
    try {
        m_items.at(local)->UpdateIID(iid);
    } catch (const std::exception&) {}
}

int Inventory::GetIID(const std::uint32_t local) const
{
    try {
        return m_items.at(local)->GetInfo().IID;
    } catch (const std::exception&) {
        return 0;
    }
}

void Inventory::ApplySpecialty(const Item::Ptr& item)
{
    m_stats[P_STR] += item->GetInit().Specialty.Stats[P_STR];
    m_stats[P_HTH] += item->GetInit().Specialty.Stats[P_HTH];
    m_stats[P_INT] += item->GetInit().Specialty.Stats[P_INT];
    m_stats[P_WIS] += item->GetInit().Specialty.Stats[P_WIS];
    m_stats[P_DEX] += item->GetInit().Specialty.Stats[P_DEX];

    m_hp    += item->GetInit().Specialty.HP;
    m_mp    += item->GetInit().Specialty.MP;
    m_def   += item->GetInit().Specialty.Defense;
    m_hit   += item->GetInit().Specialty.Hit;
    m_dodge += item->GetInit().Specialty.Dodge;
    m_absorb+= item->GetInit().Specialty.Absorb;

    if (item->GetInit().Class == IC_WEAPON)
        m_aspeed = item->GetInit().Specialty.AttackSpeed;

    m_minattack+= item->GetInit().Specialty.MinAttack;
    m_maxattack+= item->GetInit().Specialty.MaxAttack;
    m_minmagic += item->GetInit().Specialty.MinMagic;
    m_maxmagic += item->GetInit().Specialty.MaxMagic;

    m_resists[RT_CURSE]     += item->GetInit().Specialty.Resists[RT_CURSE];
    m_resists[RT_FIRE]      += item->GetInit().Specialty.Resists[RT_FIRE];
    m_resists[RT_ICE]       += item->GetInit().Specialty.Resists[RT_ICE];
    m_resists[RT_LITNING]   += item->GetInit().Specialty.Resists[RT_LITNING];
    m_resists[RT_PALSY]     += item->GetInit().Specialty.Resists[RT_PALSY];
}

void Inventory::FreeSpecialty(const Item::Ptr& item)
{
    m_stats[P_STR] -= item->GetInit().Specialty.Stats[P_STR];
    m_stats[P_HTH] -= item->GetInit().Specialty.Stats[P_HTH];
    m_stats[P_INT] -= item->GetInit().Specialty.Stats[P_INT];
    m_stats[P_WIS] -= item->GetInit().Specialty.Stats[P_WIS];
    m_stats[P_DEX] -= item->GetInit().Specialty.Stats[P_DEX];

    m_hp    -= item->GetInit().Specialty.HP;
    m_mp    -= item->GetInit().Specialty.MP;
    m_def   -= item->GetInit().Specialty.Defense;
    m_hit   -= item->GetInit().Specialty.Hit;
    m_dodge -= item->GetInit().Specialty.Dodge;
    m_absorb-= item->GetInit().Specialty.Absorb;

    if (item->GetInit().Class == IC_WEAPON)
        m_aspeed = 0;

    m_minattack-= item->GetInit().Specialty.MinAttack;
    m_maxattack-= item->GetInit().Specialty.MaxAttack;
    m_minmagic -= item->GetInit().Specialty.MinMagic;
    m_maxmagic -= item->GetInit().Specialty.MaxMagic;

    m_resists[RT_CURSE]     -= item->GetInit().Specialty.Resists[RT_CURSE];
    m_resists[RT_FIRE]      -= item->GetInit().Specialty.Resists[RT_FIRE];
    m_resists[RT_ICE]       -= item->GetInit().Specialty.Resists[RT_ICE];
    m_resists[RT_LITNING]   -= item->GetInit().Specialty.Resists[RT_LITNING];
    m_resists[RT_PALSY]     -= item->GetInit().Specialty.Resists[RT_PALSY];
}