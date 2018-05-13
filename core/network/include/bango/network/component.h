#pragma once

#include <iostream>

namespace bango { namespace network {

    class session;
    class component
    {
        session* m_session;
    protected:
        session* get_session() const;
    public:
        virtual const char* id() const = 0;

        explicit component(session* sess);
    };

}}