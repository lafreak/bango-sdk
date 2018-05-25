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


    public:

        void connect(const std::string& host, std::int32_t port);
        void when(unsigned char type, const std::function<void(packet&)>&& callback);
    };

    void client::connect(const std::string& host, std::int32_t port)
    {
        m_client->connect(host, port);
        m_client->async_read({MAX_PACKET_LENGTH, [=](const taco_read_result_t& res) {
            on_new_message(res);
        }});
    }

    void client::on_new_message(const taco_read_result_t& res)
    {
        if (res.success)
        {
            m_client->async_read({MAX_PACKET_LENGTH, [=](const taco_read_result_t& res) {
                on_new_message(res);
            }});

            auto buffer = res.buffer;

            while (((unsigned short*)buffer.data())[0] <= buffer.size())
            {
                auto size = ((unsigned short*)buffer.data())[0];
                execute(packet(std::vector<char>(buffer.begin(), buffer.begin() + size)));
                buffer.erase(buffer.begin(), buffer.begin() + size);
            }

            if (buffer.size() > 0)
                std::cerr << "packet leftover\n";
        }
        else
        {
            std::cerr << "server was closed " << m_client->get_host() << ":" << m_client->get_port() << std::endl;
            m_client->disconnect();
        }
    }

    void client::execute(packet&& p) const
    {
        auto result = m_callbacks.find(p.type());
        if (result == m_callbacks.end())
            std::cerr << "unknown packet " << (int)p.type() << std::endl;
        else
            result->second(p);            
    }

    void client::when(unsigned char type, const std::function<void(packet&)>&& callback)
    {
        m_callbacks.insert(std::make_pair(type, callback));
    }

}}