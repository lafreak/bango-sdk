#pragma once

#include <bango/processor/parser.h>

#include <unordered_map>
#include <functional>
#include <memory>
#include <stdexcept>
#include <optional>
#include <cstring>

namespace bango { namespace processor {

    template<typename T>
    class db_object
    {
    public:
        static bool Load(const char* path, std::optional<const char*> name_filter = std::nullopt) { return container::instance().load(path, name_filter); }

        static const std::unordered_map<unsigned int, const std::unique_ptr<T>>& DB() { return container::instance().db(); }

        static const T* Find(unsigned int index)
        {
            try {
                return container::instance().db().at(index).get();
            } catch (const std::out_of_range&) {
                return nullptr;
            }
        }

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

            const std::unordered_map<unsigned int, const std::unique_ptr<T>>& db() const { return m_db; }

        private:
            std::unordered_map<unsigned int, const std::unique_ptr<T>> m_db;

            container() {}
            ~container() 
            {
            }

        public:
            bool load(const char* path, std::optional<const char*> name_filter = std::nullopt)
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

                    if(name_filter.has_value() && std::strcmp(name, name_filter.value()) == 0)
                        continue;

                    T temp = {};

                    while (param.consp())
                        temp.set(param.pop());

                    // BUG: Some configs have multiple types of rows for example InitNPC npc/gennpc. Filter?
                    if (m_db.find(temp.index()) == m_db.end()) 
                        m_db.insert(std::make_pair(temp.index(), std::make_unique<T>(temp)));
                }

                return true; 
            }
        };
    };

}}