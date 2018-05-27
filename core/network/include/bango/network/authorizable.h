#pragma once

namespace bango { namespace network {

    class authorizable 
    {
        //! 32 roles 1 bit each
        int m_roles=0;

    public:
        void grant  (int roles) { m_roles |= roles;  }
        void ban    (int roles) { m_roles &= ~roles; }

        bool authorized(int roles) const { return !((~m_roles) & roles); }

    };

}}