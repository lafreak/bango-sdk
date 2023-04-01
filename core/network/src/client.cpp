#include <bango/network/client.h>

#include <cassert>

using namespace bango::network;

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

        remaining_buffer.insert(remaining_buffer.end(), res.buffer.begin(), res.buffer.end());

        while (true)
        {
            // not enough packet data to process yet
            if (remaining_buffer.size() < 3)
                break;

            // not enough packet buffer data than declared in the packet yet
            if (((unsigned short*)remaining_buffer.data())[0] > remaining_buffer.size())
                break;

            if (((unsigned short*)remaining_buffer.data())[0] < 3)
            {
                std::cerr << "wrong packet size: " << ((unsigned short*)remaining_buffer.data())[0] << "\n";
                // TODO: Kick?
                remaining_buffer.clear();
                return;
            }

            execute(packet(std::vector<char>(remaining_buffer.begin(), remaining_buffer.begin() + ((unsigned short*)remaining_buffer.data())[0])));
            remaining_buffer.erase(remaining_buffer.begin(), remaining_buffer.begin() + ((unsigned short*)remaining_buffer.data())[0]);

            if (remaining_buffer.size() == 0)
                break;
        }
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