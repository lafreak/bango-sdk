#pragma once

#include <bango/processor/parser.h>

#include <unordered_map>
#include <functional>
#include <memory>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <string>
#include <filesystem>

namespace bango { namespace processor {

    template<typename T>
    class db_object
    {
    public:
        static bool Load(const std::string& path, const std::string& file_name, const std::string& name_filter = "")
        {
            std::filesystem::path full_file_path = (std::filesystem::path) path / (std::filesystem::path) file_name;
            return container::instance().load(full_file_path.c_str(), name_filter);
        }

        static const std::unordered_map<unsigned int, const std::unique_ptr<T>>& DB() { return container::instance().db(); }

        static const T* Find(unsigned int index)
        {
            const auto& db = container::instance().db();
            const auto it = db.find(index);
            if (it != db.end())
                return it->second.get();

            return nullptr;
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
            bool load(const char* path, const std::string& name_filter = "")
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

                    std::string name = (const char*) param.pop();

                    // filter if required
                    if(!name_filter.empty())
                    {
                        // ignore ()
                        std::string skip_chars = "()";
                        name.erase(std::remove_if(name.begin(), name.end(),
                            [&skip_chars](const char &c) {
                                return skip_chars.find(c) != std::string::npos;
                            }),
                            name.end());

                        // ignore uppercase
                        std::transform(name.begin(), name.end(), name.begin(),
                            [](unsigned char c){ return std::tolower(c); });

                        if (name != name_filter)
                            continue;
                    }

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