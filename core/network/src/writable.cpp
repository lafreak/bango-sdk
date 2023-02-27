#include <bango/network/writable.h>

namespace bango { namespace network {

    void writable::write(const packet& p) const
    {
        try {
            m_client->async_write({p.buffer(), nullptr});
        } catch (tacopie::tacopie_error& err) {
            // Ignore errors in case the socket is already disconnected.
        }
    }

    void writable::write(unsigned char type) const
    {
        write(packet(type));
    }

    void writable::write(unsigned char type, const char* format, ...) const
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
    
}}