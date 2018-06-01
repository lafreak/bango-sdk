#pragma once

#include <bango/processor/parser.h>

#include <map>
#include <functional>

namespace bango { namespace processor {

    template<typename T>
    class db_object
    {
    public:
        static bool Load(const char* path) { return container::instance().load(path); }

        static const std::map<unsigned int, const T*>& DB() { return container::instance().db(); }

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

            const std::map<unsigned int, const T*>& db() const { return m_db; }

        private:
            std::map<unsigned int, const T*> m_db;

            container() {}
            ~container() 
            {
                for (auto& pair : m_db)
                    delete pair.second;
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
                    return false; // if lisp?

                while (var.consp())
                {
                    lisp::var param = var.pop();

                    auto name = (const char*) param.pop();

                    T* record = new T{};

                    while (param.consp())
                        record->set(param.pop());

                    // BUG: Some configs have multiple types of rows for example InitNPC npc/gennpc. Filter?
                    //! Prevents duplicate indices.
                    if (m_db.insert(std::make_pair(record->index(), record)).second == false)
                        delete record;
                }

                return true; 
            }
        };
    };

}}