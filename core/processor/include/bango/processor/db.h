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
      struct ci_less
      {
          // case-independent (ci) compare_less binary function
          struct nocase_compare
          {
              bool operator()(const unsigned char &c1, const unsigned char &c2) const
              {
                  return tolower(c1) < tolower(c2);
              }
          };
          bool operator()(const std::string &s1, const std::string &s2) const
          {
              return std::lexicographical_compare(s1.begin(), s1.end(), // source range
                                                  s2.begin(), s2.end(), // dest range
                                                  nocase_compare());    // comparison
          }
      };

      std::map<std::string, unsigned int, ci_less> m_attributes;

      unsigned int attribute(const char *param) const
      {
          auto result = m_attributes.find(param);
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

            T* record = new T{};

            while (param.consp())
                process(*record, param.pop());

            m_db.insert(std::make_pair(record->index(), record));
        }

        return true;
    }

}}