#pragma once

#include <bango/network/packet.h>

#include <tacopie/tacopie>
#include <cstdarg>
#include <memory>

namespace bango { namespace network {

    typedef std::shared_ptr<tacopie::tcp_client> taco_client_t;

    class writable
    {
    protected:
        const std::shared_ptr<tacopie::tcp_client> m_client;

    public:
        writable() : m_client(std::make_shared<tacopie::tcp_client>()) {}
        
        explicit
        writable(const std::shared_ptr<tacopie::tcp_client>& client) 
            : m_client(client) {}

        void write(unsigned char type) const;
        void write(const packet& p) const;
        void write(unsigned char type, const char* format, ...) const;
    };

}}