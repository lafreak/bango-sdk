#pragma once

#include <bango/processor/parser.h>

#include <map>
#include <functional>
#include <memory>

namespace bango { namespace processor {

    template<typename T>
    class db_object
    {
    public:
        static bool Load(const char* path) { return container::instance().load(path); }

        static const std::map<unsigned int, const std::shared_ptr<T>>& DB() { return container::instance().db(); }

    private:
        virtual void set(lisp::var param) = 0;
        virtual unsigned int index() const = 0;

        class container
        {
        public:
            static container& instance()
            {
                static container instance;
                return instance;
            }

            const std::map<unsigned int, const std::shared_ptr<T>>& db() const { return m_db; }

        private:
            std::map<unsigned int, const std::shared_ptr<T>> m_db;

            container() {}
            ~container() 
            {
            }

        public:
            bool load(const char* path)
            {
                XParser parser;
                XFileEx file;

                if (!file.Open(path))
                    return false;

                lisp::var var = parser.Load(&file);

                if (var.errorp())
                    return false; // if listp?

                while (var.consp())
                {
                    lisp::var param = var.pop();

                    auto name = (const char*) param.pop();

                    T temp = {};

                    while (param.consp())
                        temp.set(param.pop());

                    // BUG: Some configs have multiple types of rows for example InitNPC npc/gennpc. Filter?
                    if (m_db.find(temp.index()) == m_db.end()) 
                        m_db.insert(std::make_pair(temp.index(), std::make_shared<T>(temp)));
                }

                return true; 
            }
        };
    };

}}