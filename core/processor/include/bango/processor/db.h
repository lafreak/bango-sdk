#pragma once

#include <bango/processor/parser.h>

#include <map>
#include <string>
#include <algorithm>
#include <iostream>

namespace bango { namespace processor {

    template<typename T, typename K>
    class db
    {
        std::map<unsigned int, T*> m_db;
    protected:
        std::map<std::string, unsigned int> m_attributes;
        
        unsigned int attribute(const char* param) const {
            std::string str = param;
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
            auto result = m_attributes.find(str);
            if (result != m_attributes.end()) 
                return result->second;
            return 0;
        }

    public:

        static bool load(const char* path) { return instance()._load(path); }
        static T* find(unsigned int index) { return instance()._find(index); }

    protected:
        virtual void process(T& t, lisp::var param) = 0;

        bool    _load(const char* path);
        T*      _find(unsigned int index) const { return m_db.at(index); }

        static K& instance()
        {
            static K instance;
            return instance;
        }

         db() {  }
        ~db() {
            for (auto& pair : m_db)
                delete pair.second;
        }
    };

    template<typename T, typename K>
    bool db<T,K>::_load(const char* path)
    {
        XParser parser;
        XFileEx file;

        if (!file.Open(path))
            return false;

        lisp::var var = parser.Load(&file);

        if (var.errorp())
            return false;

        while (var.consp())
        {
            lisp::var param = var.pop();

            auto name = (const char*) param.pop();

            T* record = new T;

            while (param.consp())
                process(*record, param.pop());

            m_db.insert(std::make_pair(record->index(), record));
        }

        return true;
    }

}}