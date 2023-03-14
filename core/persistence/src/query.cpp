#include <bango/persistence/query.h>

namespace bango { namespace persistence {

    query::query(Connection_T conn, const char* q)
    {
        m_pstatement = Connection_prepareStatement(conn, "%s", q);
        m_result = nullptr;
        m_index = 1;
    }

    query& query::set(int value)
    {
        if (m_result) return *this;
        PreparedStatement_setInt(m_pstatement, m_index++, value);
        return *this;
    }

    query& query::set(const char *value) 
    {
        if (m_result) return *this;
        PreparedStatement_setString(m_pstatement, m_index++, value);
        return *this;
    }

    query& query::set(const std::string& value)
    {
        return set(value.c_str());
    }

    int query::get_int() 
    {
        if (!m_result) return 0;
        return ResultSet_getInt(m_result, m_index++);
    }

    int query::get_int(const char* column) 
    {
        if (!m_result) return 0;
        return ResultSet_getIntByName(m_result, column);
    }

    long long query::get_long() 
    {
        if (!m_result) return 0;
        return ResultSet_getLLong(m_result, m_index++);
    }

    long long query::get_long(const char* column) 
    {
        if (!m_result) return 0;
        return ResultSet_getLLongByName(m_result, column);
    }

    std::string query::get_str() 
    {
        if (!m_result) return "";
        return ResultSet_getString(m_result, m_index++);
    }

    std::string query::get_str(const char* column) 
    {
        if (!m_result) return "";
        return ResultSet_getStringByName(m_result, column);
    }

    void query::execute_query() 
    {
        if (m_result) return;

        m_index = 1;
        m_result = PreparedStatement_executeQuery(m_pstatement);
    }

    long long int query::execute()
    {
        if (m_result) return 0;

        m_index = 1;
        PreparedStatement_execute(m_pstatement);
        return PreparedStatement_rowsChanged(m_pstatement);
    }

    bool query::next() 
    {
        if (!m_result) return false;

        m_index = 1;
        return ResultSet_next(m_result);
    }

}}