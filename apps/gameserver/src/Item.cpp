#include "Item.h"

#include <cassert>

void InitItem::set(bango::processor::lisp::var param)
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
    }

    WearId = FindWearId(Kind);
}

Item::Item(const InitItem* init, const ITEMINFO& info)
    : m_init(init), m_info(info)
{
    static unsigned int g_max_local_id=0;
    m_local_id = ++g_max_local_id;
}

Item::operator bango::network::packet() const
{
    bango::network::packet p;

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
            m_wear_items[init->WearId] = result.first->second;
            AddVisibleIndex(info.Index);
        }

        if (init->Class == IC_RIDE && info.Info & ITEM_PUTON)
        {
            m_ride = result.first->second;
            AddVisibleIndex(info.Index);
        }

        return result.first->second;

    } catch (const std::out_of_range&) {
        std::cerr << "Non existing item index: " << info.Index << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return nullptr;
}

void Inventory::Reset()
{
    m_items.clear();
    m_ride = nullptr;

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

    return item;
}

Inventory::operator bango::network::packet() const
{
    bango::network::packet p(S2C_ITEMINFO);
    p << (unsigned short) m_items.size();

    for (const auto& pair : m_items)
        p << (bango::network::packet) *(pair.second.get());

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