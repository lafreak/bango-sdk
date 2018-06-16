#include <iostream>
#include <cassert>
#include <map>
#include <sstream>

#include "Socket.h"
#include "World.h"

#include "CommandDispatcher.h"

using namespace bango::network;
using namespace bango::space;
using namespace bango::processor;

class GameManager
{
    // BUG: Very inefficient.
    void UserByUID(unsigned int uid, const std::function<void(const std::shared_ptr<Player>&)>&& callback) const
    {
        for (auto& session : Socket::GameServer().sessions())
        {
            if (session.second->GetUID() == uid)
                callback(session.second);
        }
    }

public:
    GameManager()
    {
        //!
        //! DBServer -> GameServer
        //!

        Socket::DBClient().when(D2S_LOGIN, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::shared_ptr<Player>& user) {
                user->assign(User::CAN_REQUEST_SECONDARY);
                user->write(p.change_type(S2C_ANS_LOGIN));
            });
        });

        // TODO: Bad naming, its not authorized
        Socket::DBClient().when(D2S_AUTHORIZED, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::shared_ptr<Player>& user) {
                user->assign(User::AUTHORIZED);
                user->SetAID(p.pop<int>());
            });
        });

        Socket::DBClient().when(D2S_SEC_LOGIN, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::shared_ptr<Player>& user) {
                user->assign(User::CAN_REQUEST_SECONDARY);
                user->write(p.change_type(S2C_SECOND_LOGIN));
            });
        });

        Socket::DBClient().when(D2S_PLAYER_INFO, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::shared_ptr<Player>& user) {
                user->write(p.change_type(S2C_PLAYERINFO));
                user->assign(User::LOBBY);
            });
        });

        Socket::DBClient().when(D2S_DELPLAYERINFO, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::shared_ptr<Player>& user) {
                user->write(p.change_type(S2C_DELPLAYERINFO));
            });
        });
        
        Socket::DBClient().when(D2S_ANS_NEWPLAYER, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::shared_ptr<Player>& user) {
                user->write(p.change_type(S2C_ANS_NEWPLAYER));
            });
        });

        Socket::DBClient().when(D2S_LOADPLAYER, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::shared_ptr<Player>& user) {
                if (p.pop<char>())
                {
                    user->write(S2C_MESSAGE, "b", MSG_NOTEXISTPLAYER);
                    return;
                }

                // BUG: Player might log in by the time packet arrived?
                user->OnLoadPlayer(p);
            });
        });

        Socket::DBClient().when(D2S_LOADITEMS, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::shared_ptr<Player>& user) {
                user->OnLoadItems(p);
            });
        });

        Socket::DBClient().when(D2S_UPDATEITEMIID, [&](packet& p) {
            UserByUID(p.pop<unsigned int>(), [&](const std::shared_ptr<Player>& user) {
                auto local = p.pop<unsigned int>();
                auto iid = p.pop<int>();
                user->UpdateItemIID(local, iid);
            });
        });
    }
};

int main()
{
    GameManager manager;

    InitItem    ::Load("Config/InitItem.txt");
    InitNPC     ::Load("Config/InitNPC.txt");

    using namespace std::placeholders;

    Socket::GameServer().on_connected(          std::bind(&Player::OnConnected,     _1));
    Socket::GameServer().on_disconnected(       std::bind(&Player::OnDisconnected,  _1));

    Socket::GameServer().when(C2S_CONNECT,      std::bind(&User::OnConnect,         _1, _2));
    Socket::GameServer().when(C2S_ANS_CODE,     std::bind(&User::OnCodeAnswer,      _1, _2));
    Socket::GameServer().when(C2S_LOGIN,        std::bind(&User::OnLogin,           _1, _2));
    Socket::GameServer().when(C2S_SECOND_LOGIN, std::bind(&User::OnSecondaryLogin,  _1, _2));
    Socket::GameServer().when(C2S_NEWPLAYER,    std::bind(&User::OnNewPlayer,       _1, _2));
    Socket::GameServer().when(C2S_DELPLAYER,    std::bind(&User::OnDeletePlayer,    _1, _2));
    Socket::GameServer().when(C2S_RESTOREPLAYER,std::bind(&User::OnRestorePlayer,   _1, _2));
    Socket::GameServer().when(C2S_LOADPLAYER,   std::bind(&User::OnLoadPlayer,      _1, _2));

    Socket::GameServer().when(C2S_REST,         std::bind(&Player::OnRest,          _1, _2));
    Socket::GameServer().when(C2S_START,        std::bind(&Player::OnStart,         _1, _2));
    Socket::GameServer().when(C2S_RESTART,      std::bind(&Player::OnRestart,       _1, _2));
    Socket::GameServer().when(C2S_GAMEEXIT,     std::bind(&Player::OnExit,          _1, _2));
    Socket::GameServer().when(C2S_MOVE_ON,      std::bind(&Player::OnMove,          _1, _2, false));
    Socket::GameServer().when(C2S_MOVE_END,     std::bind(&Player::OnMove,          _1, _2, true ));
    Socket::GameServer().when(C2S_CHATTING,     std::bind(&Player::OnChatting,      _1, _2));
    Socket::GameServer().when(C2S_PUTONITEM,    std::bind(&Player::OnPutOnItem,     _1, _2));
    Socket::GameServer().when(C2S_PUTOFFITEM,   std::bind(&Player::OnPutOffItem,    _1, _2));
    Socket::GameServer().when(C2S_USEITEM,      std::bind(&Player::OnUseItem,       _1, _2));
    Socket::GameServer().when(C2S_TRASHITEM,    std::bind(&Player::OnTrashItem,     _1, _2));

    CommandDispatcher::Register("/get",         std::bind(&Player::OnGetItem,       _1, _2));

    World::OnAppear     (std::bind(&Player::OnCharacterAppear,      _1, _2));
    World::OnDisappear  (std::bind(&Player::OnCharacterDisappear,   _1, _2));
    World::OnMove       (std::bind(&Player::OnCharacterMove,        _1, _2, _3, _4, _5, _6));

    Socket::GameServer().grant(
    {
        {C2S_LOGIN,             User::CAN_REQUEST_PRIMARY},
        {C2S_SECOND_LOGIN,      User::CAN_REQUEST_SECONDARY},

        {C2S_NEWPLAYER,         User::LOBBY},
        {C2S_DELPLAYER,         User::LOBBY},
        {C2S_RESTOREPLAYER,     User::LOBBY},
        {C2S_LOADPLAYER,        User::LOBBY},

        {C2S_START,             User::LOADING},

        {C2S_RESTART,           User::INGAME},
        {C2S_MOVE_ON,           User::INGAME},
        {C2S_MOVE_END,          User::INGAME},
        {C2S_CHATTING,          User::INGAME},
        {C2S_PUTONITEM,         User::INGAME},
        {C2S_PUTOFFITEM,        User::INGAME},
        {C2S_USEITEM,           User::INGAME},
        {C2S_TRASHITEM,         User::INGAME},
    });

    //Socket::GameServer().restrict({});

    World::SpawnNpcs();

    try 
    {
        Socket::DBClient().connect("localhost", 2999);
        Socket::GameServer().start("localhost", 3000);
    } 
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::cin.get();
    return 0;
}