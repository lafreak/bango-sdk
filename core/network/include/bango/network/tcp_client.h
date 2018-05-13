#pragma once

#include <tacopie/tacopie>
#include <bango/network/packet.h>
#include <map>
#include <stdarg.h>

namespace bango { namespace network {

    class tcp_client
    {
        tacopie::tcp_client m_client;

        void on_new_message(const tacopie::tcp_client::read_result& res);

        std::map<unsigned char, std::function<void(packet&)>> m_callbacks;
        void execute(packet&& p);

    public:
        tcp_client();

        bool connect(const std::string& host, std::int32_t port);

        void when(unsigned char type, std::function<void(packet&)> callback);

        void write(unsigned char type);
        void write(unsigned char type, const char* format, ...);
        void write(const packet& p);
    };

}}