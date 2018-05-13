#include <bango/persistence/connection.h>

#include <iostream>

namespace bango { namespace persistence {

    connection::connection(ConnectionPool_T p)
    {
        m_conn = ConnectionPool_getConnection(p);
    }

    connection::~connection()
    {
        Connection_close(m_conn);
    }

    query connection::create_query(const char* q)
    {
        return query(m_conn, q);
    }

}}