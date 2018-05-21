#include <bango/network/packet.h>

namespace bango { namespace network {

    void packet::push_str(const char* str)
    {
        auto size = strlen(str)+1;

        if ((m_end-m_buffer) + size > MAX_PACKET_LENGTH)
            throw std::runtime_error("packet overflow");

        std::memcpy(m_end, str, size);
        m_end += size;

        reset_size();
    }

    std::string packet::pop_str()
    {
        auto size = strlen(m_begin)+1;
        
        if (m_end-m_begin < size)
            throw std::runtime_error("packet no more data");

        std::string result(m_begin);
        m_begin += size;

        reset_size();

        return result;
    }

    void packet::merge(const packet& p)
    {
        if ((m_end-m_buffer) + p.size()-3 > MAX_PACKET_LENGTH)
            throw std::runtime_error("packet overflow");

        std::memcpy(m_end, p.begin(), p.size()-3);
        m_end += p.size()-3;

        reset_size();
    }

}}