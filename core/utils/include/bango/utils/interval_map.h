#include <assert.h>
#include <map>
#include <limits>
#include <cstdint>
#include <iostream>

namespace bango::utils
{
template<typename V>
class interval_map
{

	V m_val_begin;
	std::map<std::uint32_t, V> m_map;
    std::uint32_t max;
public:
	interval_map()
		: m_val_begin()
        , max(0)
	{}

	void assign(std::uint32_t const& keyBegin, std::uint32_t const& keyEnd, V const& val) 
	{
		if (!(keyBegin < keyEnd)) 
			return;

		auto currentKeyBeginValue = this->operator[](keyBegin);
		auto currentKeyEndValue = this->operator[](keyEnd);

		auto beginRemoveIterator = m_map.upper_bound(keyBegin);
		auto endRemoveIterator = m_map.upper_bound(keyEnd);

		if (!(currentKeyBeginValue == val))
		{
			auto [iterator, success] = m_map.insert_or_assign(keyBegin, val);
			if (iterator != m_map.begin() && (--iterator)->second == val)
				--beginRemoveIterator;
		}

		if (!(currentKeyEndValue == val))
		{
			auto [iterator, success] = m_map.insert_or_assign(keyEnd, currentKeyEndValue);
            max = std::max(max, keyEnd);
			endRemoveIterator = iterator;
		}

		if (beginRemoveIterator != m_map.end() && (endRemoveIterator == m_map.end() || beginRemoveIterator->first < endRemoveIterator->first))
			m_map.erase(beginRemoveIterator, endRemoveIterator);
	}

    std::uint32_t GetMaxKey() const 
    {
        return max;
    }

	V const& operator[](std::uint32_t const& key) const {
		auto it = m_map.upper_bound(key);
		if (it == m_map.begin()) {
			return m_val_begin;
		}
		else {
			return (--it)->second;
		}
	}

	auto GetMap() const
	{
		return m_map;
	}

	std::size_t size() const
	{
		return m_map.size();
	}
};

} // namespace bango::utils