#pragma once

#include <bango/network/writable.h>

#include <functional>
#include <map>
#include <iostream>

namespace bango { namespace network {

    class client : public writable
    {
        typedef tacopie::tcp_client::read_result taco_read_result_t;

        std::map<unsigned char, std::function<void(packet&)>> m_callbacks;
        void on_new_message(const taco_read_result_t& res);

        void execute(packet&& p) const;

        // possibly reserve MAX_PACKET_LENGTH for better perf
        std::vector<char> remaining_buffer;

    public:

        void connect(const std::string& host, std::int32_t port);
        void when(unsigned char type, const std::function<void(packet&)>&& callback);
    };

}}