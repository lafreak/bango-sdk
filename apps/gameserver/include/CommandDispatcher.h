#pragma once

#include <sstream>
#include <unordered_map>
#include <functional>

class Player;
class CommandDispatcher
{
public:
    class Token
    {
        constexpr static char DELIMETER = ' ';
        std::istringstream m_stream;

        std::string token() 
        {
            std::string tok;
            m_stream >> std::ws;
            std::getline(m_stream, tok, DELIMETER);
            return tok;
        }

    public:
        Token(const std::string& str) : m_stream(str) {}

        operator std::string() 
        {
            return token();
        }

        operator int()
        {
            try {
                return std::stoi(token());
            } catch (const std::exception&) {
                return 0;
            }
        }

        operator float()
        {
            try {
                return std::stof(token());
            } catch (const std::exception&) {
                return 0.0;
            }
        }
    };

    using Task = const std::function<void (Player &, Token &)>;
private:
    std::unordered_map<std::string, Task> m_commands;

    static CommandDispatcher& Get();

    CommandDispatcher(){}
    ~CommandDispatcher(){}

public:
    static void Register(const std::string& command, Task&& task);
    static void Dispatch(Player& player, const std::string& message);
};