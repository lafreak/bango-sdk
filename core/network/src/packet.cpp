#include <bango/network/packet.h>

#include <algorithm>
#include <string.h>

namespace bango { namespace network {

    packet::packet(const std::vector<char>& buffer): m_buffer(buffer)
    {
        if (size_ex() != m_buffer.size())
        {
            std::cerr << __FUNCTION__ << ": Packet size mismatch" << std::endl;
        }
    }

    void packet::push_str(const std::string& str)
    {
        if (str.size() + 1 + size() > MAX_PACKET_LENGTH)
        {
            std::cerr << __FUNCTION__ << ": Packet overflow" << std::endl;
            return;
        }

        std::copy(str.begin(), str.end(), std::back_inserter(m_buffer));

        reset_size();
        push<char>(0);
    }

    void packet::push_str(const char* str)
    {
        if (strlen(str) + 1 + size() > MAX_PACKET_LENGTH)
        {
            std::cerr << __FUNCTION__ << ": Packet overflow" << std::endl;
            return;
        }

        std::copy(str, str+strlen(str), std::back_inserter(m_buffer));

        reset_size();
        push<char>(0);
    }

    std::string packet::pop_str()
    {
        if (size() == 3)
        {
            std::cerr << __FUNCTION__ << ": Packet is empty" << std::endl;
            return "";
        }

        auto result = std::find(m_buffer.begin() + 3, m_buffer.end(), 0);
        if (result == m_buffer.end())
        {
            std::cerr << __FUNCTION__ << ": No string" << std::endl;
            return "";
        }

        auto value = std::string(m_buffer.begin() + 3, result);
        m_buffer.erase(m_buffer.begin() + 3, result + 1);

        reset_size();
        return value;
    }

    void packet::dump() const
    {
        printf("Type [%d]\nSize[%d]\nContent: ", type(), size());
        for (auto c : m_buffer)
            printf("%d ", (unsigned char)c);
        printf("\n");
    }

    void packet::merge(const packet& p)
    {
        if (size() + p.size() - 3 > MAX_PACKET_LENGTH)
        {
            std::cerr << __FUNCTION__ << ": Packet overflow" << std::endl;
            return;
        }

        m_buffer.insert(m_buffer.end(), p.buffer().begin()+3, p.buffer().end());

        reset_size();
    }

}}