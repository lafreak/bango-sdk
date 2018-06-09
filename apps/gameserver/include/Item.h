#pragma once

#include <bango/network/packet.h>
#include <bango/processor/db.h>

#include <inix.h>

#include <map>
#include <memory>

struct InitItem : public bango::processor::db_object<InitItem>
{
    unsigned short
        Index;

    unsigned int 
        Class,
        Level;

    int 
        WearId,
        Kind;

    unsigned char
        Endurance,
        LimitLevel,
        LimitClass=PC_ALL,
        RidingType;

    bool
        Plural,
        Wearable;

    unsigned int index() const { return Index; }

    virtual void set(bango::processor::lisp::var param) override;
};

class Item
{
    // TODO: const InitItem& ? const unique ptr & ?
    const InitItem*     m_init;
    ITEMINFO            m_info;

    std::uint32_t m_local_id;

public:
    typedef std::shared_ptr<Item> Ptr;

    Item(const InitItem* init, const ITEMINFO& info);

    const std::uint32_t GetLocalID()    const { return m_local_id; }

    const InitItem& GetInit() const { return *m_init; }
    const ITEMINFO& GetInfo() const { return  m_info; }

    void    UpdateNum(unsigned int num)    { m_info.Num = num; }
    void    UpdateIID(int iid)             { m_info.IID = iid; }

    void AddInfo(unsigned int info) { m_info.Info |= info; }
    void SubInfo(unsigned int info) { m_info.Info &= ~info; }

    operator bango::network::packet() const;
};

class Inventory
{
protected:
    Inventory() { Reset(); }

private:
    std::map<unsigned int, const Item::Ptr> m_items;

    EQUIPMENT m_equipment;

    Item::Ptr m_wear_items[WS_LAST];

    Item::Ptr m_ride;

    void AddVisibleIndex    (unsigned short index);
    void RemoveVisibleIndex (unsigned short index);

public:

    //! Creates an item out of item info and inserts it.
    //! Returns inserted item instance or nullptr if error occured.
    const Item::Ptr Insert(const ITEMINFO& info);  

    //! Resets inventory state and makes it empty.
    void Reset();

    //! Search interface.
    const Item::Ptr FindByIndex(unsigned short index) const;
    const Item::Ptr FindByLocalID(const std::uint32_t local) const;

    //! Removes item with specific Local ID if possible.
    //! Returns false if operation was impossible or true otherwise.
    bool Trash(const std::uint32_t local);

    //! Equips item with specific Local ID if possible.
    //! Returns the item or nullptr if an error occured.
    const Item::Ptr PutOn(const std::uint32_t local);

    //! Puts off itemw ith specific LocalID if possible.
    //! Returns the item or nullptr if an error occured.
    const Item::Ptr PutOff(const std::uint32_t local);

    //! Returns visible equipped items (indexes).
    const EQUIPMENT& GetEquipment() const { return m_equipment; }

    //! Casts whole inventory (group of items) to packet.
    operator bango::network::packet() const;

    //! Finds and updates item IID based on given Local ID.
    void UpdateItemIID(const std::uint32_t local, int iid);

    //! Finds item IID based on given Local ID.
    int GetIID(const std::uint32_t local) const;
};
