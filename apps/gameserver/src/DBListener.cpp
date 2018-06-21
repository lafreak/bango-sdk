#include "DBListener.h"
#include "Socket.h"
#include "Player.h"

using namespace bango::network;

void DBListener::OnLogin(packet& p)
{
    if (auto user = Socket::FindUserByUID(p.pop<unsigned int>()))
    {
        user->assign(User::CAN_REQUEST_SECONDARY);
        user->write(p.change_type(S2C_ANS_LOGIN));
    }
}

void DBListener::OnAuthorized(packet& p)
{
    if (auto user = Socket::FindUserByUID(p.pop<unsigned int>()))
    {
        user->assign(User::AUTHORIZED);
        user->SetAID(p.pop<int>());
    }
}

void DBListener::OnSecondaryLogin(packet& p)
{
    if (auto user = Socket::FindUserByUID(p.pop<unsigned int>()))
    {
        user->assign(User::CAN_REQUEST_SECONDARY);
        user->write(p.change_type(S2C_SECOND_LOGIN));
    }
}

void DBListener::OnPlayerInfo(packet& p)
{
    if (auto user = Socket::FindUserByUID(p.pop<unsigned int>()))
    {
        user->write(p.change_type(S2C_PLAYERINFO));
        user->assign(User::LOBBY);
    }
}

void DBListener::OnDeletePlayerInfo(packet& p)
{
    if (auto user = Socket::FindUserByUID(p.pop<unsigned int>()))
    {
        user->write(p.change_type(S2C_DELPLAYERINFO)); 
    }
}

void DBListener::OnNewPlayerAnswer(packet& p)
{
    if (auto user = Socket::FindUserByUID(p.pop<unsigned int>()))
    {
        user->write(p.change_type(S2C_ANS_NEWPLAYER));
    }
}

void DBListener::OnLoadPlayer(packet& p)
{
    if (auto user = Socket::FindUserByUID(p.pop<unsigned int>()))
    {
        if (p.pop<char>())
        {
            user->write(S2C_MESSAGE, "b", MSG_NOTEXISTPLAYER);
            return;
        }

        // BUG: Player might log in by the time packet arrived?
        user->OnLoadPlayer(p);
    }
}

void DBListener::OnLoadItems(packet& p)
{
    if (auto user = Socket::FindUserByUID(p.pop<unsigned int>()))
    {
        user->OnLoadItems(p);
    }
}

void DBListener::OnUpdateItemIID(packet& p)
{
    if (auto user = Socket::FindUserByUID(p.pop<unsigned int>()))
    {
        auto local = p.pop<unsigned int>();
        auto iid = p.pop<int>();
        user->UpdateItemIID(local, iid);
    }
}
