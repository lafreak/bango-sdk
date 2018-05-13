#include <bango/network/session.h>

#include <string.h>

namespace bango { namespace network {

    // non thread-safe
    unsigned int session::g_id = 0;

    session::session(const std::shared_ptr<tacopie::tcp_client>& client)
        : m_client(client), m_id(g_id++)
    {
    }

    void session::write(unsigned char type)
    {
        write(packet(type));
    }

    void session::write(unsigned char type, const char* format, ...)
    {
        packet p(type);

        va_list va;
        va_start(va, format);
        
        for (size_t i = 0, n = strlen(format); i < n; i++)
        {
            switch (format[i])
            {
            case 'b':
                p.push<unsigned char>(va_arg(va, int));
                break;
            case 'w':
                p.push<unsigned short>(va_arg(va, int));
                break;
            case 'd':
                p.push<unsigned int>(va_arg(va, int));
                break;
            case 'I':
                p.push<unsigned long>(va_arg(va, long));
                break;
            case 's':
                p.push_str(va_arg(va, const char*));
                break;
            }
        }

        va_end(va);

        write(p);
    }

    void session::write(const packet& p)
    {
        m_client->async_write({p.buffer(), nullptr});
    }

}}