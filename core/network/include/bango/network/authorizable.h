#pragma once

namespace bango { namespace network {

    class authorizable 
    {
        //! 32 roles 1 bit each
        int m_roles=0;

    public:
        void assign (int roles) { m_roles |=  roles; }
        void deny   (int roles) { m_roles &= ~roles; }

        bool authorized(int required, int restricted=0) const { 
            return !((~m_roles) & required) && !(m_roles & restricted); 
        }
    };

}}