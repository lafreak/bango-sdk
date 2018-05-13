#pragma once

#include <vector>
#include <iostream>

//#define MAX_PACKET_LENGTH 1024
#define MAX_PACKET_LENGTH 32768

namespace bango { namespace network {

    class packet
    {
        std::vector<char> m_buffer;

        void reset_size() { ((unsigned short*)(m_buffer.data()))[0] = size(); }

        unsigned short size_ex() const { return ((unsigned short*)(m_buffer.data()))[0]; }

    public:
        packet() { m_buffer = {3, 0, 0}; }
        packet(unsigned char type) { m_buffer = {3, 0, (char)type}; }
        packet(const std::vector<char>& buffer);

        const std::vector<char>& buffer() const { return m_buffer; }

        unsigned char type() const { return (unsigned char) m_buffer[2]; }
        unsigned short size() const { return (unsigned short) m_buffer.size(); }

        packet& change_type(unsigned char type) { m_buffer[2] = (char)type; return *this; }

        template<typename T>
        void push(const T& value);

        template<typename T>
        T pop();

        void push_str(const std::string& str);
        void push_str(const char* str);
        std::string pop_str();

        template<typename T>
        friend packet& operator>> (packet& lhs, T& rhs) { rhs = lhs.pop<T>(); return lhs; }
        friend packet& operator>> (packet& lhs, std::string& rhs) { rhs = lhs.pop_str(); return lhs; }

        friend packet& operator<< (packet& lhs, std::string& rhs) { lhs.push_str(rhs); return lhs; }
        friend packet& operator<< (packet& lhs, const char* rhs) { lhs.push_str(rhs); return lhs; }

        void dump() const;

        void merge(const packet& p);
        friend packet& operator<< (packet& lhs, const packet& rhs) { lhs.merge(rhs); return lhs; }

        bool valid() const { return size_ex() == size(); }
    };

    template<typename T>
    inline void packet::push(const T& value)
    {
        if (size() + sizeof(T) > MAX_PACKET_LENGTH)
        {
            std::cerr << __FUNCTION__ << ": Packet overflow" << std::endl;
            return;
        }

        m_buffer.insert(m_buffer.end(), (char*)&value, ((char*)&value)+sizeof(T));
        
        reset_size();
    }

    template<typename T>
    inline T packet::pop()
    {
        if (size() < sizeof(T) + 3)
        {
            std::cerr << __FUNCTION__ << ": Packet is empty [" << (int)type() << "]" << size() << "/" << sizeof(T) << std::endl;
            return T();
        }

        T result = *(T*)(m_buffer.data() + 3);

        m_buffer.erase(m_buffer.begin() + 3, m_buffer.begin() + 3 + sizeof(T));

        reset_size();

        return result;
    }

}}