#pragma once

#include <tacopie/tacopie>

#include <bango/network/packet.h>
#include <bango/network/component.h>
#include <stdarg.h>
#include <unordered_map>
#include <typeinfo>

namespace bango { namespace network {

    class session
    {
        static unsigned int g_id;
        unsigned int m_id;


        const std::shared_ptr<tacopie::tcp_client> m_client;

        std::unordered_map<const std::type_info*, const std::shared_ptr<component>> m_components;

    public:
        session(const std::shared_ptr<tacopie::tcp_client>& client);

        unsigned int get_id() const { return m_id; }
        const std::string& get_host() const { return m_client->get_socket().get_host(); }

        void write(unsigned char type);
        void write(unsigned char type, const char* format, ...);
        void write(const packet& p);

        template<class T>
        const std::shared_ptr<T> get();
    };

    template<class T>
    inline const std::shared_ptr<T> session::get()
    {
        auto result = m_components.find(&typeid(T));
        if (result == m_components.end())
        {
            auto it = m_components.insert(std::pair<const std::type_info*, const std::shared_ptr<T>>(
                &typeid(T),
                std::make_shared<T>(this)
            ));
            
            return std::dynamic_pointer_cast<T>(it.first->second);
        }
        else
        {
            return std::dynamic_pointer_cast<T>(result->second);
        }
    }

}}