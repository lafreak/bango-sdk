#pragma once

#include <zdb.h>

#include <bango/persistence/query.h>

#include <string>

namespace bango { namespace persistence {

    class connection
    {
        Connection_T m_conn;

    public:
        connection(ConnectionPool_T p);
        ~connection();

        query create_query(const char* q);
    };

}}