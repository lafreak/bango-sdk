#include "User.h"

#include "Socket.h"
#include "World.h"

using namespace bango::network;


void User::OnConnect(packet& p)
{
    // ProtocolVersion, Code, TimeStamp, TimeStampStart, System, Event, ServerID, Age, Country
    write(S2C_CODE, "dbdddIbbb", 0, 0, 604800, 0, 0, 0, 0, 18, N_EN);
}

void User::OnCodeAnswer(packet& p)
{
}

void User::OnLogin(packet& p)
{
    deny(User::CAN_REQUEST_PRIMARY);
    p << GetUID();
    Socket::DBClient().write(p.change_type(S2D_LOGIN));
}

void User::OnSecondaryLogin(packet& p)
{
    deny(User::CAN_REQUEST_SECONDARY);
    p << GetCredentials();
    Socket::DBClient().write(p.change_type(S2D_SECONDARY_LOGIN));
}

void User::OnNewPlayer(packet& p)
{
    packet copy = p;
    
    auto name =     p.pop_str();
    auto job =      p.pop<unsigned char>();
    auto str =      p.pop<unsigned short>();
    auto hth =      p.pop<unsigned short>();
    auto int_ =     p.pop<unsigned short>();
    auto wis =      p.pop<unsigned short>();
    auto dex =      p.pop<unsigned short>();
    auto face =     p.pop<unsigned char>();
    auto hair =     p.pop<unsigned char>();

    if (name.empty() || name.size() > 14)
    {
        write(S2C_ANS_NEWPLAYER, "b", NA_ERROR);
        return;
    }

    if (job >= CLASS_NUM)
    {
        write(S2C_ANS_NEWPLAYER, "b", NA_WRONGCLASS);
        return;
    }

    if (str + hth + int_ + wis + dex != 5)
    {
        write(S2C_ANS_NEWPLAYER, "b", NA_WRONGPROPERTY);
        return;
    }

    if (hair > 6 || face > 6)
    {
        write(S2C_ANS_NEWPLAYER, "b", NA_WRONGPROPERTY);
        return;
    }

    copy << GetCredentials();

    Socket::DBClient().write(copy.change_type(S2D_NEWPLAYER));
}

void User::OnDeletePlayer(packet& p)
{
    p << GetCredentials();
    Socket::DBClient().write(p.change_type(S2D_DELPLAYER));
}

void User::OnRestorePlayer(packet& p)
{
    p << GetCredentials();
    Socket::DBClient().write(p.change_type(S2D_RESTOREPLAYER));
}

void User::OnLoadPlayer(packet& p)
{
    assign(User::LOADING);
    deny(User::LOBBY);
    
    p << GetCredentials();
    Socket::DBClient().write(p.change_type(S2D_LOADPLAYER));
}