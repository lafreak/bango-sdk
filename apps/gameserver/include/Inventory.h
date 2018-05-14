#pragma once

#include "Item.h"

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

class Inventory
{
    std::vector<std::shared_ptr<Item>> m_items;

public:
    void Insert(const std::shared_ptr<Item>& item)
    {
        m_items.push_back(item);
    }

    void FindByIndex(unsigned short index, std::function<void(const std::shared_ptr<Item>&)> callback)
    {
        auto result = std::find_if(m_items.begin(), m_items.end(), [index](const std::shared_ptr<Item>& i) -> bool { return i->GetIndex() == index; });
        if (result != m_items.end())
            callback(*result);
    }

    void FindByIID(int iid, std::function<void(const std::shared_ptr<Item>&)> callback)
    {
        auto result = std::find_if(m_items.begin(), m_items.end(), [iid](const std::shared_ptr<Item>& i) -> bool { return i->GetIID() == iid; });
        if (result != m_items.end())
            callback(*result);
    }
};