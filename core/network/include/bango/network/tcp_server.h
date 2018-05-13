#pragma once

#include <tacopie/tacopie>
#include <bango/network/packet.h>
#include <bango/network/session.h>
#include <map>

namespace bango { namespace network {

    class tcp_server
    {
        tacopie::tcp_server m_server;

        std::function<void(const std::shared_ptr<session>&)> m_on_connected_cb;
        std::function<void(const std::shared_ptr<session>&)> m_on_disconnected_cb;

        std::map<unsigned char, std::function<void(const std::shared_ptr<session>&, packet&)>> m_callbacks;
        std::map<int*, const std::shared_ptr<session>> m_sessions;
        
        std::recursive_mutex m_sessions_rmtx;

        const std::shared_ptr<session>& add(const std::shared_ptr<tacopie::tcp_client>& client);
        void remove(const std::shared_ptr<tacopie::tcp_client>& client);
        const std::shared_ptr<session>& find(const std::shared_ptr<tacopie::tcp_client>& client);

        void execute(const std::shared_ptr<session>& s, packet&& p);

        void on_new_message(const std::shared_ptr<tacopie::tcp_client>& client, const tacopie::tcp_client::read_result& res);

    public:
        tcp_server();
        ~tcp_server();

        bool start(const std::string& host, std::int32_t port);

        void when(unsigned char type, std::function<void(const std::shared_ptr<session>&, packet&)> callback);
        void by_id(unsigned int id, std::function<void(const std::shared_ptr<session>&)> callback);

        void on_connected(std::function<void(const std::shared_ptr<session>&)>);
        void on_disconnected(std::function<void(const std::shared_ptr<session>&)>);
    };

}}