#pragma once

#include <bango/network/writable.h>
#include <bango/network/authorizable.h>

#include <functional>
#include <map>
#include <memory>
#include <iostream>
#include <algorithm>

namespace bango { namespace network {

    template<class T>
    class server
    {
        typedef std::shared_ptr<tacopie::tcp_client> taco_client_t;
        typedef tacopie::tcp_client::read_result taco_read_result_t;

        tacopie::tcp_server m_server;

        std::map<int*, const std::shared_ptr<T>> m_sessions;
        std::recursive_mutex m_sessions_rmtx;

        std::function<void(const std::shared_ptr<T>&)> m_on_connected;
        std::function<void(const std::shared_ptr<T>&)> m_on_disconnected;

        void on_new_message(const taco_client_t& client, const taco_read_result_t& res);

        const std::shared_ptr<T>&   insert  (const taco_client_t& client);
        void                        remove  (const taco_client_t& client);
        const std::shared_ptr<T>&   find    (const taco_client_t& client);//BUG: Its not thread safe ?

        std::map<unsigned char, std::pair<const std::function<void(const std::shared_ptr<T>&, packet&)>,std::pair<int,int>>> m_callbacks;
        std::map<unsigned char, int> m_granted_roles;
        std::map<unsigned char, int> m_restricted_roles;

        void execute(const std::shared_ptr<T>& session, packet&& p) const;

    public:

        void start(const std::string& host, std::int32_t port);
        void when(unsigned char type, const std::function<void(const std::shared_ptr<T>&, packet&)>&& callback);
        void on_connected(const std::function<void(const std::shared_ptr<T>&)>&& callback);
        void on_disconnected(const std::function<void(const std::shared_ptr<T>&)>&& callback);
        void for_each(const std::function<void(const std::shared_ptr<T>&)>&& callback);

        void grant      (const std::map<unsigned char, int>&& roles);
        void restrict   (const std::map<unsigned char, int>&& roles);

        std::uint16_t get_online() const;
    };

    template<class T>
    void server<T>::start(const std::string& host, std::int32_t port)
    {
        m_server.start(host, port, [&](const taco_client_t& client) -> bool {
            //TODO: Dont even insert if maximum users exceeded. 1024 limited by taco.

            client->async_read({MAX_PACKET_LENGTH, [=](const taco_read_result_t& res) {
                on_new_message(client, res);
            }});

            auto& session = insert(client);

            if (m_on_connected)
                m_on_connected(session);

            return false;
        });
    }

    template<class T>
    void server<T>::on_new_message(const taco_client_t& client, const taco_read_result_t& res)
    {
        auto& session = find(client);

        if (res.success)
        {
            client->async_read({MAX_PACKET_LENGTH, [=](const taco_read_result_t& res) {
                on_new_message(client, res);
            }});

            auto buffer = res.buffer; //?

            while (((unsigned short*)buffer.data())[0] <= buffer.size())
            {
                auto size = ((unsigned short*)buffer.data())[0];
                execute(session, packet(std::vector<char>(buffer.begin(), buffer.begin() + size)));
                buffer.erase(buffer.begin(), buffer.begin() + size);
            }

            if (buffer.size() > 0)
                std::cerr << "packet leftover\n";
        }
        else
        {
            if (m_on_disconnected)
                m_on_disconnected(session);
            remove(client);
            client->disconnect();
        }
    }

    template<class T>
    const std::shared_ptr<T>& server<T>::insert(const taco_client_t& client)
    {
        std::lock_guard<std::recursive_mutex> lock(m_sessions_rmtx);

        auto result = m_sessions.insert(std::make_pair(
            (int*) client.get(),
            std::make_unique<T>(client)
        ));

        if (!result.second)
            throw std::runtime_error("duplicate session");

        return result.first->second;
    }

    template<class T>
    void server<T>::remove(const taco_client_t& client)
    {
        std::lock_guard<std::recursive_mutex> lock(m_sessions_rmtx);

        if (m_sessions.erase((int*) client.get()) == 0)
            throw std::runtime_error("session removal of non existing client");
    }

    template<class T>
    const std::shared_ptr<T>& server<T>::find(const taco_client_t& client)
    {
        std::lock_guard<std::recursive_mutex> lock(m_sessions_rmtx);

        // BUG: returned value may be deleted from server later on
        return m_sessions[(int*) client.get()];
    }

    template<class T>
    std::uint16_t server<T>::get_online() const
    {
        //std::lock_guard<std::recursive_mutex> lock(m_sessions_rmtx);
        // Is it really data race free?

        return m_sessions.size();
    }

    template<class T>
    void server<T>::execute(const std::shared_ptr<T>& session, packet&& p) const
    {
        auto result = m_callbacks.find(p.type());
        if (result == m_callbacks.end())
            std::cerr << "unknown packet " << (int)p.type() << std::endl;
        else if (!session->authorized(result->second.second.first, result->second.second.second))
            std::cerr << "session unauthorized " << (int)p.type() << std::endl;
        else
            result->second.first(session, p);            
    }

    template<class T>
    void server<T>::on_connected(const std::function<void(const std::shared_ptr<T>&)>&& callback)
    {
        m_on_connected = callback;
    }

    template<class T>
    void server<T>::on_disconnected(const std::function<void(const std::shared_ptr<T>&)>&& callback)
    {
        m_on_disconnected = callback;
    }

    template<class T>
    void server<T>::when(unsigned char type, const std::function<void(const std::shared_ptr<T>&, packet&)>&& callback)
    {
        //m_callbacks[type] = callback;
        m_callbacks.insert(std::make_pair(type, std::make_pair(callback, std::make_pair(0,0))));
    }

    template<class T>
    void server<T>::for_each(const std::function<void(const std::shared_ptr<T>&)>&& callback)
    {
        std::lock_guard<std::recursive_mutex> lock(m_sessions_rmtx);

        for (auto& session : m_sessions)
            callback(session.second);
    }
    
    template<class T>
    void server<T>::grant(const std::map<unsigned char, int>&& roles)
    {
        for (auto& pair : roles) {
            auto event = m_callbacks.find(pair.first);
            if (event != m_callbacks.end())
                event->second.second.first = pair.second;
        }
    }

    template<class T>
    void server<T>::restrict(const std::map<unsigned char, int>&& roles)
    {
        for (auto& pair : roles) {
            auto event = m_callbacks.find(pair.first);
            if (event != m_callbacks.end())
                event->second.second.second = pair.second;
        }
    }

}}