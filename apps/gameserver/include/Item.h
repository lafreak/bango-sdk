#pragma once

#include <inix/protocol.h>
#include <inix/common.h>
#include <inix/structures.h>

class Item
{
    ITEMINFO m_data;
public:
    Item(const ITEMINFO& data): m_data(data) {}
    Item(const ITEMINFO&& data): m_data(data) {}

    unsigned short  GetIndex()  const { return m_data.Index; }
    int             GetIID()    const { return m_data.IID; }
};