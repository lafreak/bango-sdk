#include "DBListener.h"
#include "Socket.h"
#include "Player.h"

using namespace bango::network;

void DBListener::OnLogin(packet& p)
{
    auto uid = p.pop<unsigned int>();

    Socket::GameServer().for_each([&](const std::shared_ptr<Player>& user) {
        if (user->GetUID() == uid)
        {
            user->assign(User::CAN_REQUEST_SECONDARY);
            user->write(p.change_type(S2C_ANS_LOGIN));
        }
    });
}

void DBListener::OnAuthorized(packet& p)
{
    auto uid = p.pop<unsigned int>();
    auto aid = p.pop<int>();

    Socket::GameServer().for_each([&](const std::shared_ptr<Player>& user) {
        if (user->GetUID() == uid)
        {
            user->assign(User::AUTHORIZED);
            user->SetAID(aid);
        }
    });
}

void DBListener::OnSecondaryLogin(packet& p)
{
    auto uid = p.pop<unsigned int>();

    Socket::GameServer().for_each([&](const std::shared_ptr<Player>& user) {
        if (user->GetUID() == uid)
        {
            user->assign(User::CAN_REQUEST_SECONDARY);
            user->write(p.change_type(S2C_SECOND_LOGIN));
        }
    });
}

void DBListener::OnPlayerInfo(packet& p)
{
    auto uid = p.pop<unsigned int>();

    Socket::GameServer().for_each([&](const std::shared_ptr<Player>& user) {
        if (user->GetUID() == uid)
        {
            user->write(p.change_type(S2C_PLAYERINFO));
            user->assign(User::LOBBY);
        }
    });
}

void DBListener::OnDeletePlayerInfo(packet& p)
{
    auto uid = p.pop<unsigned int>();

    Socket::GameServer().for_each([&](const std::shared_ptr<Player>& user) {
        if (user->GetUID() == uid)
        {
            user->write(p.change_type(S2C_DELPLAYERINFO)); 
        }
    });
}

void DBListener::OnNewPlayerAnswer(packet& p)
{
    auto uid = p.pop<unsigned int>();

    Socket::GameServer().for_each([&](const std::shared_ptr<Player>& user) {
        if (user->GetUID() == uid)
        {
            user->write(p.change_type(S2C_ANS_NEWPLAYER));
        }
    });
}

void DBListener::OnLoadPlayer(packet& p)
{
    auto uid = p.pop<unsigned int>();
    auto answer = p.pop<char>();

    Socket::GameServer().for_each([&](const std::shared_ptr<Player>& user) {
        if (user->GetUID() == uid)
        {
            if (answer)
            {
                user->write(S2C_MESSAGE, "b", MSG_NOTEXISTPLAYER);
                return;
            }

            // BUG: Player might log in by the time packet arrived?
            user->OnLoadPlayer(p);
        }
    });
}

void DBListener::OnLoadItems(packet& p)
{
    auto uid = p.pop<unsigned int>();

    Socket::GameServer().for_each([&](const std::shared_ptr<Player>& user) {
        if (user->GetUID() == uid)
        {
            user->OnLoadItems(p);
        }
    });
}

void DBListener::OnUpdateItemIID(packet& p)
{
    auto uid = p.pop<unsigned int>();
    auto local = p.pop<unsigned int>();
    auto iid = p.pop<int>();

    Socket::GameServer().for_each([&](const std::shared_ptr<Player>& user) {
        if (user->GetUID() == uid)
        {
            user->GetInventory().UpdateItemIID(local, iid);
            //user->UpdateItemIID(local, iid);
        }
    });
}
