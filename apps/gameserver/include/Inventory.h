#pragma once

#include <unordered_map>
#include <memory>
#include <cstdint>

#include <bango/network/packet.h>
#include <bango/processor/db.h>

#include <inix.h>

struct InitItem : public bango::processor::db_object<InitItem>
{
    unsigned short
        Index=0;

    unsigned int 
        Class=0,
        Level=0;

    int 
        WearId=0,
        Kind=0;

    unsigned char
        Endurance=0,
        LimitLevel=1,
        LimitClass=PC_ALL,
        RidingType=0;

    bool
        Plural=false,
        Wearable=false;

    struct
    {
        unsigned int Stats[5] ={0,};
        unsigned int HP=0, MP=0, Defense=0, Hit=0, Dodge=0, Absorb=0;
        unsigned int Resists[5] ={0,};
        unsigned int AttackSpeed=0, MinAttack=0, MaxAttack=0, MinMagic=0, MaxMagic=0;
    } Specialty;

    unsigned int index() const { return Index; }

    virtual void set(bango::processor::lisp::var param) override;
};

class Item
{
    // TODO: const InitItem& ? const unique ptr & ?
    const InitItem*     m_init;
    ITEMINFO            m_info;

public:
    typedef std::shared_ptr<Item> Ptr;
    typedef uint32_t local_id_t;

    Item(const InitItem* init, const ITEMINFO& info);

    local_id_t GetLocalID()   const { return m_local_id; }

    const InitItem& GetInit() const { return *m_init; }
    const ITEMINFO& GetInfo() const { return  m_info; }

    void    UpdateNum(unsigned int num)    { m_info.Num = num; }
    void    UpdateIID(int iid)             { m_info.IID = iid; }

    void AddInfo(unsigned int info) { m_info.Info |= info; }
    void SubInfo(unsigned int info) { m_info.Info &= ~info; }

    operator bango::network::packet() const;

private:
    local_id_t          m_local_id;
};

class Inventory
{
public:
    Inventory() { Reset(); }

private:
    std::unordered_map<Item::local_id_t, const Item::Ptr> m_items;

    EQUIPMENT m_equipment;

    Item::Ptr m_wear_items[WS_LAST];

    Item::Ptr m_ride=nullptr;

    void AddVisibleIndex    (unsigned short index);
    void RemoveVisibleIndex (unsigned short index);

    void ApplySpecialty (const Item::Ptr&);
    void FreeSpecialty  (const Item::Ptr&);

    std::uint16_t m_stats[5];
    std::uint16_t m_hp=0, m_mp=0, m_def=0, m_hit=0, m_dodge=0, m_absorb=0;
    std::uint16_t m_resists[5];
    std::uint16_t m_aspeed=0, m_minattack=0, m_maxattack=0, m_minmagic=0, m_maxmagic=0;

public:
    std::uint16_t GetHealth()       const { return m_stats[P_HTH]; }
    std::uint16_t GetStrength()     const { return m_stats[P_STR]; }
    std::uint16_t GetInteligence()  const { return m_stats[P_INT]; }
    std::uint16_t GetWisdom()       const { return m_stats[P_WIS]; }
    std::uint16_t GetDexterity()    const { return m_stats[P_DEX]; }

    std::uint16_t GetHP()       const { return m_hp; }
    std::uint16_t GetMP()       const { return m_mp; }
    std::uint16_t GetDefense()  const { return m_def; }
    std::uint16_t GetHit()      const { return m_hit; }
    std::uint16_t GetDodge()    const { return m_dodge; }
    std::uint16_t GetAbsorb()   const { return m_absorb; }

    std::uint16_t GetResist(std::uint8_t type) const { return m_resists[type]; }

    std::uint16_t GetMinAttack() const { return m_minattack; }
    std::uint16_t GetMaxAttack() const { return m_maxattack; }
    std::uint16_t GetMinMagic()  const { return m_minmagic; }
    std::uint16_t GetMaxMagic()  const { return m_maxmagic; }

    std::uint16_t GetAttackSpeed()  const { return m_aspeed; }

    bool HasWeapon()                const { return GetAttackSpeed() > 0; }

public:

    //! Creates an item out of item info and inserts it.
    //! Returns inserted item instance or nullptr if error occured.
    const Item::Ptr Insert(const ITEMINFO& info);  

    //! Resets inventory state and makes it empty.
    void Reset();

    //! Search interface.
    const Item::Ptr FindByIndex(unsigned short index) const;
    const Item::Ptr FindByLocalID(const Item::local_id_t local) const;

    //! Removes item with specific Local ID if possible.
    //! Returns false if operation was impossible or true otherwise.
    bool Trash(const Item::local_id_t local);

    //! Equips item with specific Local ID if possible.
    //! Returns the item or nullptr if an error occured.
    const Item::Ptr PutOn(const Item::local_id_t local);

    //! Puts off itemw ith specific LocalID if possible.
    //! Returns the item or nullptr if an error occured.
    const Item::Ptr PutOff(const Item::local_id_t local);

    //! Returns visible equipped items (indexes).
    const EQUIPMENT& GetEquipment() const { return m_equipment; }

    //! Casts whole inventory (group of items) to packet.
    operator bango::network::packet() const;

    //! Finds and updates item IID based on given Local ID.
    void UpdateItemIID(const Item::local_id_t local, int iid);

    //! Finds item IID based on given Local ID.
    int GetIID(const Item::local_id_t local) const;
};
