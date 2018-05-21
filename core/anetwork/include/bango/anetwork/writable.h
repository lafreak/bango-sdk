#pragma once

#include <bango/anetwork/packet.h>

#include <tacopie/tacopie>
#include <cstdarg>

namespace bango { namespace anetwork {

    class writable
    {
    protected:
        tacopie::tcp_client m_client;

    public:
        void write(unsigned char type);
        void write(const packet& p);
        void write(unsigned char type, char* format, ...);
    };

    void writable::write(const packet& p)
    {
        m_client.async_write({p.buffer(), nullptr});
    }

    void writable::write(unsigned char type)
    {
        write(packet(type));
    }

    void writable::write(unsigned char type, char* format, ...)
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