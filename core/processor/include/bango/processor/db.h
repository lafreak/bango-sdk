#pragma once

#include <bango/processor/parser.h>

#include <map>

namespace bango { namespace processor {

    template<typename T>
    class db_object
    {
    public:
        static bool Load(const char* path) { return container::instance().load(path); }
        static T*   Find(unsigned int index) { return container::instance().find(index); }

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

        private:
            std::map<unsigned int, T*> m_db;

            container() {}
            ~container() 
            {
                for (auto& pair : m_db)
                    delete pair.second;
            }

        public:
            T* find(unsigned int index) { return m_db.at(index); }
            
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

                    // BUG: Memory leak when already existing index.
                    // BUG: File stays is in use.
                    // BUG: Some configs have multiple types of rows for example InitNPC npc/gennpc.
                    m_db.insert(std::make_pair(record->index(), record));
                }

                return true; 
            }
        };
    };

}}