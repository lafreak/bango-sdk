#include <iostream>
#include <cassert>
#include <map>
#include <sstream>
#include <future>
#include <chrono>

#include "Socket.h"
#include "World.h"

#include "CommandDispatcher.h"
#include "DBListener.h"

using namespace bango::network;
using namespace bango::space;
using namespace bango::processor;

int main()
{
    InitItem    ::Load("Config/InitItem.txt");
    InitNPC     ::Load("Config/InitNPC.txt");
    InitMonster ::Load("Config/InitMonster.txt");

    using namespace std::placeholders;

    Socket::GameServer().on_connected(              std::bind(&Player::OnConnected,     _1));
    Socket::GameServer().on_disconnected(           std::bind(&Player::OnDisconnected,  _1));

    Socket::GameServer().when(C2S_CONNECT,          std::bind(&User::OnConnect,         _1, _2));
    Socket::GameServer().when(C2S_ANS_CODE,         std::bind(&User::OnCodeAnswer,      _1, _2));
    Socket::GameServer().when(C2S_LOGIN,            std::bind(&User::OnLogin,           _1, _2));
    Socket::GameServer().when(C2S_SECOND_LOGIN,     std::bind(&User::OnSecondaryLogin,  _1, _2));
    Socket::GameServer().when(C2S_NEWPLAYER,        std::bind(&User::OnNewPlayer,       _1, _2));
    Socket::GameServer().when(C2S_DELPLAYER,        std::bind(&User::OnDeletePlayer,    _1, _2));
    Socket::GameServer().when(C2S_RESTOREPLAYER,    std::bind(&User::OnRestorePlayer,   _1, _2));
    Socket::GameServer().when(C2S_LOADPLAYER,       std::bind(&User::OnLoadPlayer,      _1, _2));

    Socket::GameServer().when(C2S_START,            std::bind(&Player::OnStart,             _1, _2));
    Socket::GameServer().when(C2S_RESTART,          std::bind(&Player::OnRestart,           _1, _2));
    Socket::GameServer().when(C2S_GAMEEXIT,         std::bind(&Player::OnExit,              _1, _2));
    Socket::GameServer().when(C2S_MOVE_ON,          std::bind(&Player::OnMove,              _1, _2, false));
    Socket::GameServer().when(C2S_MOVE_END,         std::bind(&Player::OnMove,              _1, _2, true ));
    Socket::GameServer().when(C2S_CHATTING,         std::bind(&Player::OnChatting,          _1, _2));
    Socket::GameServer().when(C2S_PUTONITEM,        std::bind(&Player::OnPutOnItem,         _1, _2));
    Socket::GameServer().when(C2S_PUTOFFITEM,       std::bind(&Player::OnPutOffItem,        _1, _2));
    Socket::GameServer().when(C2S_USEITEM,          std::bind(&Player::OnUseItem,           _1, _2));
    Socket::GameServer().when(C2S_TRASHITEM,        std::bind(&Player::OnTrashItem,         _1, _2));
    Socket::GameServer().when(C2S_REST,             std::bind(&Player::OnRest,              _1, _2));
    Socket::GameServer().when(C2S_TELEPORT,         std::bind(&Player::OnTeleportAnswer,    _1, _2));
    Socket::GameServer().when(C2S_UPDATEPROPERTY,   std::bind(&Player::OnUpdateProperty,    _1, _2));
    Socket::GameServer().when(C2S_PLAYER_ANIMATION, std::bind(&Player::OnPlayerAnimation,   _1, _2));
    Socket::GameServer().when(C2S_ATTACK,           std::bind(&Player::OnAttack,            _1, _2));

    Socket::DBClient().when(D2S_LOGIN,          std::bind(&DBListener::OnLogin,             _1));
    Socket::DBClient().when(D2S_AUTHORIZED,     std::bind(&DBListener::OnAuthorized,        _1));
    Socket::DBClient().when(D2S_SEC_LOGIN,      std::bind(&DBListener::OnSecondaryLogin,    _1));
    Socket::DBClient().when(D2S_PLAYER_INFO,    std::bind(&DBListener::OnPlayerInfo,        _1));
    Socket::DBClient().when(D2S_DELPLAYERINFO,  std::bind(&DBListener::OnDeletePlayerInfo,  _1));
    Socket::DBClient().when(D2S_ANS_NEWPLAYER,  std::bind(&DBListener::OnNewPlayerAnswer,   _1));
    Socket::DBClient().when(D2S_LOADPLAYER,     std::bind(&DBListener::OnLoadPlayer,        _1));
    Socket::DBClient().when(D2S_LOADITEMS,      std::bind(&DBListener::OnLoadItems,         _1));
    Socket::DBClient().when(D2S_UPDATEITEMIID,  std::bind(&DBListener::OnUpdateItemIID,     _1));

    CommandDispatcher::Register("/get",         std::bind(&Player::OnGetItem,       _1, _2));
    CommandDispatcher::Register("/move2",       std::bind(&Player::OnMoveTo,        _1, _2));
    
    CommandDispatcher::Register("/mob", [&](Player* player, CommandDispatcher::Token& token) {
        //World::Add(new Monster((int)token, player->GetX()+10, player->GetY()+10, player->GetMap()));
    
        int index = token;

        try {
            World::Add(new Monster(InitMonster::DB().at(index), player->GetX()+10, player->GetY()+10, player->GetMap()));
        } catch (const std::exception&) {
            std::cout << "Monster Index doesnt exist " << index << std::endl;
        }
    });

    World::OnAppear     (std::bind(&Player::OnCharacterAppear,      _1, _2, _3));
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
        {C2S_TELEPORT,          User::INGAME},
        {C2S_REST,              User::INGAME},
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

    using namespace std::chrono_literals;

    std::promise<void> done;
    std::shared_future<void> done_future(done.get_future());

    auto ticker = std::async(std::launch::async, [&]{
        std::future_status status;
        do {
            status = done_future.wait_for(1s);
            if (status == std::future_status::timeout) {
                World::ForEachPlayer([](Player* player) {
                    player->Tick();
                });

                // World::ForEachNpc([](NPC* npc) {
                //     npc->Tick();
                // });
            }
        } while (status != std::future_status::ready);
    });

    std::cin.get();
    done.set_value();

    World::Cleanup();

    return 0;
}