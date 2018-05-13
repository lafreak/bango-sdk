#include <bango/persistence/pool.h>

#include <iostream>

namespace bango { namespace persistence {

    pool::pool()
    {
        m_url = nullptr;
        m_pool = nullptr;
    }

    pool::~pool()
    {
        if (m_pool)
            ConnectionPool_free(&m_pool);

        if (m_url)
            URL_free(&m_url);
    }

    bool pool::connect(const std::string& host, const std::string& port, const std::string& user, const std::string& password, const std::string& schema)
    {
        std::string url = "mysql://"+user+":"+password+"@"+host+":"+port+"/"+schema;

        m_url = URL_new(url.c_str());
        m_pool = ConnectionPool_new(m_url);

        TRY
        {
            ConnectionPool_start(m_pool);
        }
        CATCH(SQLException)
        {
            std::cerr << Exception_frame.message << std::endl;
            return false;
        }
        END_TRY;

        return true;
    }

    connection pool::get()
    {
        return connection(m_pool);
    }

}}
