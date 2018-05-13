#pragma once

#include <zdb.h>
#include <string>

namespace bango { namespace persistence {

    class query
    {
        PreparedStatement_T m_pstatement;
        ResultSet_T m_result;
        
        int m_index;

    public:
        query(Connection_T conn, const char* q);

        query& set(int value);
        query& set(const char *value);
        query& set(const std::string& value);

        template<typename T>
        friend query& operator<< (query& lhs, T rhs) { lhs.set(rhs); return lhs; }

        int get_int();
        int get_int(const char* column);
        long long get_long();
        long long get_long(const char* column);
        std::string get_str();
        std::string get_str(const char* column);

        void execute_query();
        long long int execute();
        bool next();

    };

}}