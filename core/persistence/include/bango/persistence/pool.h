#pragma once

#include <bango/persistence/query.h>
#include <bango/persistence/connection.h>

namespace bango { namespace persistence {

    class pool
    {
        ConnectionPool_T m_pool;
        URL_T m_url;

    public:
        pool();
        ~pool();

        bool connect(
            const std::string& host, 
            const std::string& port, 
            const std::string& user, 
            const std::string& password, 
            const std::string& schema);

        connection get();
    };

}}