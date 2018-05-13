#include <bango/network/tcp_server.h>

#include <iostream>
#include <algorithm>

namespace bango { namespace network {

    tcp_server::tcp_server()
    {
        m_on_connected_cb       = [](const std::shared_ptr<session>&){};
        m_on_disconnected_cb    = [](const std::shared_ptr<session>&){};
    }

    tcp_server::~tcp_server()
    {
    }
    
    bool tcp_server::start(const std::string& host, std::int32_t port)
    {
        try
        {
            m_server.start(host, port, [&](const std::shared_ptr<tacopie::tcp_client>& client) -> bool {
                client->async_read({
                    MAX_PACKET_LENGTH,
                    [=](const tacopie::tcp_client::read_result& res) {
                        on_new_message(client, res);
                    }
                });

                m_on_connected_cb(add(client));

                return false;
            });
        }
        catch (const tacopie::tacopie_error& e)
        {
            std::cerr << __FUNCTION__ << ": " << e.what() << std::endl;
            return false;
        }

        return true;
    }

    void tcp_server::on_new_message(const std::shared_ptr<tacopie::tcp_client>& client, const tacopie::tcp_client::read_result& res)
    {
        auto sess = find(client);

        if (res.success)
        {
            client->async_read({
                MAX_PACKET_LENGTH,
                [=](const tacopie::tcp_client::read_result& res) -> void {
                    on_new_message(client, res);
                }
            });

            auto buffer = res.buffer;

            while (((unsigned short*)buffer.data())[0] <= buffer.size())
            {
                auto size = ((unsigned short*)buffer.data())[0];
                //execute(sess, std::make_shared<packet>(std::vector<char>(buffer.begin(), buffer.begin() + size)));
                execute(sess, packet(std::vector<char>(buffer.begin(), buffer.begin() + size)));
                buffer.erase(buffer.begin(), buffer.begin() + size);
            }

            if (buffer.size() > 0)
                std::cerr << __FUNCTION__ << ": Packet leftover" << std::endl;
        }
        else
        {
            m_on_disconnected_cb(sess);
            remove(client);
            client->disconnect();
        }
    }

    const std::shared_ptr<session>& tcp_server::add(const std::shared_ptr<tacopie::tcp_client>& client)
    {
        std::lock_guard<std::recursive_mutex> lock(m_sessions_rmtx);

        auto result = m_sessions.insert(std::pair<int*, const std::shared_ptr<session>>(
            (int*)client.get(),
            std::make_shared<session>(client)
        ));

        if (!result.second)
            std::cerr << __FUNCTION__ << ": Duplicate session" << std::endl;

        return result.first->second;
    }

    void tcp_server::remove(const std::shared_ptr<tacopie::tcp_client>& client)
    {
        std::lock_guard<std::recursive_mutex> lock(m_sessions_rmtx);

        if (m_sessions.erase((int*)client.get()) == 0)
            std::cerr << __FUNCTION__ << ": Session doesnt exist" << std::endl;
    }

    const std::shared_ptr<session>& tcp_server::find(const std::shared_ptr<tacopie::tcp_client>& client)
    {
        std::lock_guard<std::recursive_mutex> lock(m_sessions_rmtx);

        return m_sessions[(int*)client.get()];
    }

    void tcp_server::when(unsigned char type, std::function<void(const std::shared_ptr<session>&, packet&)> callback)
    {
        m_callbacks[type] = callback;
    }

    void tcp_server::by_id(unsigned int id, std::function<void(const std::shared_ptr<session>&)> callback)
    {
        std::lock_guard<std::recursive_mutex> lock(m_sessions_rmtx);

        auto result = std::find_if(m_sessions.begin(), m_sessions.end(), [id](const std::pair<int*, const std::shared_ptr<session>>& pair) -> bool {
            return pair.second->get_id() == id;
        });
        
        if (result != m_sessions.end())
            callback(result->second);
    }

    void tcp_server::execute(const std::shared_ptr<session>& s, packet&& p)
    {
        auto result = m_callbacks.find(p.type());
        if (result == m_callbacks.end())
        {
            std::cerr << __FUNCTION__ << ": Unknown packet " << (unsigned int) p.type() << std::endl;
            return;
        }

        result->second(s, p);
    }

    void tcp_server::on_connected(std::function<void(const std::shared_ptr<session>&)> cb)
    {
        m_on_connected_cb = cb;
    }

    void tcp_server::on_disconnected(std::function<void(const std::shared_ptr<session>&)> cb)
    {
        m_on_disconnected_cb = cb;
    }

}}