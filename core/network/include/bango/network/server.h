#pragma once

#include <bango/network/writable.h>
#include <bango/network/authorizable.h>

#include <functional>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <algorithm>
#include <list>

namespace bango { namespace network {

    template<class T>
    class server
    {
        struct client_metadata {
            // on_new_message not always collects entire chunk of data but stops in the middle.
            // This remaining buffer linked to session keeps remaining chunk which are not fully received yet.
            std::vector<char> m_remaining_buffer;
        };

        typedef std::shared_ptr<tacopie::tcp_client> taco_client_t;
        typedef tacopie::tcp_client::read_result taco_read_result_t;

        typedef std::pair<const std::shared_ptr<T>, client_metadata> session_with_meta_t;
        typedef std::shared_ptr<session_with_meta_t> session_with_meta_ptr_t;

        tacopie::tcp_server m_server;

        std::list<session_with_meta_ptr_t> m_sessions;
        std::mutex m_sessions_mtx;

        std::function<void(const std::shared_ptr<T>&)> m_on_connected;
        std::function<void(const std::shared_ptr<T>&)> m_on_disconnected;

        void on_new_message(session_with_meta_ptr_t session_with_meta_ptr, const taco_client_t& client, const taco_read_result_t& res);

        std::unordered_map<unsigned char, std::pair<const std::function<void(const std::shared_ptr<T>&, packet&)>,std::pair<int,int>>> m_callbacks;

        void execute(const std::shared_ptr<T>& session, packet&& p) const;

        std::uint16_t m_max_online=1024;
        std::function<void(const writable& client)> m_on_max_online_exceeded;

    public:
        void set_max_online(std::uint16_t max_online) { m_max_online = max_online; }
        void set_nb_workers(std::size_t worker_count) { m_server.get_io_service()->set_nb_workers(worker_count); }

        void start(const std::string& host, std::int32_t port);
        void when(unsigned char type, const std::function<void(const std::shared_ptr<T>&, packet&)>&& callback);
        void on_connected(const std::function<void(const std::shared_ptr<T>&)>&& callback);
        void on_disconnected(const std::function<void(const std::shared_ptr<T>&)>&& callback);
        void for_each(const std::function<void(const std::shared_ptr<T>&)>&& callback);

        void grant      (const std::unordered_map<unsigned char, int>&& roles);
        void restrict   (const std::unordered_map<unsigned char, int>&& roles);

        std::uint16_t get_online() const;
        void on_max_online_exceeded(const std::function<void(const writable& client)>&& callback) { m_on_max_online_exceeded = callback; }
    };

    template<class T>
    void server<T>::start(const std::string& host, std::int32_t port)
    {
        m_server.start(host, port, [&](const taco_client_t& client) -> bool 
        {
            std::lock_guard<std::mutex> lock(m_sessions_mtx);
            if (get_online() >= m_max_online)
            {
                if (m_on_max_online_exceeded)
                    m_on_max_online_exceeded(writable(client));
                return false;
            }

            // Add new session.
            m_sessions.emplace_back(std::make_shared<session_with_meta_t>(std::make_pair(std::make_shared<T>(client), client_metadata{})));
            auto& session_with_meta_ptr = m_sessions.back();

            if (m_on_connected)
                m_on_connected(session_with_meta_ptr->first);

            client->async_read({MAX_PACKET_LENGTH, [this, session_with_meta_ptr, client](const taco_read_result_t& res) {
                on_new_message(session_with_meta_ptr, client, res);
            }});

            return false;
        });
    }

    template<class T>
    void server<T>::on_new_message(session_with_meta_ptr_t session_with_meta_ptr, const taco_client_t& client, const taco_read_result_t& res)
    {
        if (res.success)
        {
            // FIXME: This is slow.
            std::vector<char> buffer;
            auto& meta = session_with_meta_ptr->second;
            if (meta.m_remaining_buffer.size() > 0) {
                buffer.reserve(meta.m_remaining_buffer.size() + res.buffer.size());
                buffer.insert(buffer.end(), meta.m_remaining_buffer.begin(), meta.m_remaining_buffer.end());
                buffer.insert(buffer.end(), res.buffer.begin(), res.buffer.end());
                meta.m_remaining_buffer.clear();
            } else {
                buffer = res.buffer;
            }

            while (((unsigned short*)buffer.data())[0] <= buffer.size())
            {
                auto size = ((unsigned short*)buffer.data())[0];
                execute(session_with_meta_ptr->first, packet(std::vector<char>(buffer.begin(), buffer.begin() + size)));
                buffer.erase(buffer.begin(), buffer.begin() + size);
            }

            if (buffer.size() > 0) {
                std::cout << "packet leftover happened\n";
                meta.m_remaining_buffer = buffer;
            }

            client->async_read({MAX_PACKET_LENGTH, [this, session_with_meta_ptr, client](const taco_read_result_t& result) {
                on_new_message(session_with_meta_ptr, client, result);
            }});
        }
        else
        {
            if (m_on_disconnected)
                m_on_disconnected(session_with_meta_ptr->first);

            // Remove the session.
            std::lock_guard<std::mutex> lock(m_sessions_mtx);
            size_t before_count = m_sessions.size();
            if (before_count == 0)
                throw std::logic_error("fatal error: session container is empty");
            m_sessions.remove_if([&session_with_meta_ptr](const auto& e) { return e->first.get() == session_with_meta_ptr->first.get(); });
            size_t after_count = m_sessions.size();
            if (after_count != before_count - 1)
                throw std::logic_error("fatal error: session has not been erased");
            client->disconnect();
        }
    }

    template<class T>
    std::uint16_t server<T>::get_online() const
    {
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
        m_callbacks.insert(std::make_pair(type, std::make_pair(callback, std::make_pair(0,0))));
    }

    template<class T>
    void server<T>::for_each(const std::function<void(const std::shared_ptr<T>&)>&& callback)
    {
        std::lock_guard<std::mutex> lock(m_sessions_mtx);

        for (auto& session : m_sessions)
            callback(session->first);
    }
    
    template<class T>
    void server<T>::grant(const std::unordered_map<unsigned char, int>&& roles)
    {
        for (auto& pair : roles) {
            auto event = m_callbacks.find(pair.first);
            if (event != m_callbacks.end())
                event->second.second.first = pair.second;
        }
    }

    template<class T>
    void server<T>::restrict(const std::unordered_map<unsigned char, int>&& roles)
    {
        for (auto& pair : roles) {
            auto event = m_callbacks.find(pair.first);
            if (event != m_callbacks.end())
                event->second.second.second = pair.second;
        }
    }

}}