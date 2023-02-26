#include <assert.h>
#include <map>
#include <limits>
#include <cstdint>
#include <iostream>

namespace bango::utils
{
template<class V>
class interval_map 
{

    std::map<std::uint32_t, V> m_map;
    std::uint32_t max;
public:

    interval_map() 
    {
        max = 0;
        m_map.insert(m_map.begin(), std::make_pair(max, V()));
    }

    void assign(std::uint32_t const& keyBegin, std::uint32_t const& keyEnd, V const& val) 
    {
        if (!(keyBegin < keyEnd)) 
        {
            return;
        }

        V valend;
        bool erase = true;

        auto start = m_map.lower_bound(keyBegin);

        auto delst = start;


        if (start != m_map.end() && start->first == keyBegin) 
        {
            valend = start->second;
            start->second = val;

            ++delst;
            if (delst == m_map.end()) 
            {
                erase = false;
            }
        }
        else {
            auto prev = start;
            --prev;
            valend = prev->second;
            if (prev->second != val) 
            {
                auto it = m_map.insert(start, std::make_pair(keyBegin, val));
                start = it;
                delst = ++it;
            }
        }

        auto end = m_map.lower_bound(keyEnd);
        auto delfs = end;
        if (delfs == delst)
            erase = false;
        if (end == m_map.end() || end->first != keyEnd) 
        {
            auto prev = end;
            --prev;
            if (prev != start) 
            {
                valend = prev->second;
            }
            if (valend != val) 
            {
                auto it = m_map.insert(end, std::make_pair(keyEnd, valend));
                max = std::max(max, keyEnd);
                delfs = it;
                if ((++it) != m_map.end() && it->second == valend)
                    m_map.erase(it);
            }
        }

        if (erase && delst != m_map.end() && delfs != m_map.end() && delst->first < delfs->first) 
        {
            m_map.erase(delst, delfs);
        }
    }

    std::uint32_t GetMaxKey() const 
    {
        return max;
    }

    V const& operator[](std::uint32_t const& key) const 
    {
        return (--m_map.upper_bound(key))->second;
    }
};

} // namespace bango::utils