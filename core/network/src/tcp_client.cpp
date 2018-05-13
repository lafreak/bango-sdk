#include <bango/network/tcp_client.h>

#include <string.h>

namespace bango { namespace network {

    tcp_client::tcp_client()
    {
    }

    bool tcp_client::connect(const std::string& host, std::int32_t port)
    {
        try
        {
            m_client.connect(host, port);
        }
        catch (const tacopie::tacopie_error& e)
        {
            std::cerr << __FUNCTION__ << ": " << e.what() << std::endl;
            return false;
        }

        m_client.async_read({
            MAX_PACKET_LENGTH,
            [=](const tacopie::tcp_client::read_result& res) {
                on_new_message(res);
            }
        });

        return true;
    }

    void tcp_client::on_new_message(const tacopie::tcp_client::read_result& res)
    {
        if (res.success)
        {
            m_client.async_read({
                MAX_PACKET_LENGTH,
                [=](const tacopie::tcp_client::read_result& res) {
                    on_new_message(res);
                }
            });

            auto buffer = res.buffer;

            while (((unsigned short*)buffer.data())[0] <= buffer.size())
            {
                auto size = ((unsigned short*)buffer.data())[0];
                execute(packet(std::vector<char>(buffer.begin(), buffer.begin() + size)));
                buffer.erase(buffer.begin(), buffer.begin() + size);
            }

            if (buffer.size() > 0)
                std::cerr << __FUNCTION__ << ": Packet leftover" << std::endl;
        }
        else
        {
            std::cout << __FUNCTION__ << ": disconnected" << std::endl;
            m_client.disconnect();
        }
    }

    void tcp_client::when(unsigned char type, std::function<void(packet&)> callback)
    {
        m_callbacks[type] = callback;
    }

    void tcp_client::execute(packet&& p)
    {
        auto result = m_callbacks.find(p.type());
        if (result == m_callbacks.end())
        {
            std::cerr << __FUNCTION__ << ": Unknown packet " << p.type() << std::endl;
            return;
        }

        result->second(p);
    }

    void tcp_client::write(unsigned char type)
    {
        write(packet(type));
    }

    void tcp_client::write(unsigned char type, const char* format, ...)
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

    void tcp_client::write(const packet& p)
    {
        m_client.async_write({p.buffer(), nullptr});
    }

    // void tcp_client::write(const std::shared_ptr<packet>& p)
    // {
    //     m_client.async_write({p->buffer(), nullptr});
    // }

}}