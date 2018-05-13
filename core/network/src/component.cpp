#include <bango/network/component.h>

namespace bango { namespace network {

    component::component(session* sess)
        : m_session(sess)
    {
    }

    session* component::get_session() const
    {
        return m_session;
    }
    
}}