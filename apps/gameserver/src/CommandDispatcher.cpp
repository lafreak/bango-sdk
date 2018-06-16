#include "CommandDispatcher.h"

CommandDispatcher& CommandDispatcher::Get()
{
    static CommandDispatcher instance;
    return instance;
}

void CommandDispatcher::Register(const std::string& command, Task&& task)
{
    Get().m_commands.insert(std::make_pair(command, task));
}

void CommandDispatcher::Dispatch(Player* player, const std::string& message)
{
    Token token(message);
    std::string lookup = token;
    
    auto result = Get().m_commands.find(lookup);
    if (result != Get().m_commands.end())
        result->second(player, token);
}