#include <iostream>
#include <cassert>
#include <map>
#include <sstream>
#include <future>
#include <chrono>
#include <cstdint>
#include <string>

#include "spdlog/spdlog.h"

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

#include "Socket.h"
#include "World.h"
#include "BeheadableMonster.h"
#include "RegularMonster.h"
#include "Spawn.h"
#include "Skill.h"

#include "CommandDispatcher.h"
#include "DBListener.h"

#include <bango/utils/random.h>
#include <bango/network/writable.h>

using namespace bango::network;
using namespace bango::utils;

int main(int argc, char** argv)
{
    CLI::App app{"KalOnline Game Server (c) Bango Emu"};

    std::string db_address{"0.0.0.0"};
    std::uint16_t db_port = 2999;
    std::string game_address{"0.0.0.0"};
    std::uint16_t game_port = 3000;
    std::size_t processing_threads = 8;  // TODO: Default value should depend on nproc
    std::string config_path = "./Config";

    app.add_option("--db_address", db_address, "DB server address");
    app.add_option("--db_port", db_port, "DB server port");
    app.add_option("--game_address", game_address, "Game server address");
    app.add_option("--game_port", game_port, "Game server port");
    app.add_option("--processing_threads", processing_threads, "Number of processing threads for incoming packets from players");
    app.add_option("--config_path", config_path, "Path to server configuration files");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("[%H:%M:%S.%e] [%l] [tid %t] %v");

    try
    {
        InitItem        ::Load(config_path, "InitItem.txt");
        InitNPC         ::Load(config_path, "InitNPC.txt");
        GenMonster      ::Load(config_path, "GenMonster.txt");
        Group           ::Load(config_path, "ItemGroup.txt", "group");
        ItemGroup       ::Load(config_path, "ItemGroup.txt", "itemgroup");
        InitMonster     ::Load(config_path, "InitMonster.txt");
        InitSkill       ::Load(config_path, "InitSkill.txt");
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }


    using namespace std::placeholders;

    Socket::GameServer().set_max_online(1024);
    Socket::GameServer().set_nb_workers(processing_threads);
    Socket::GameServer().on_max_online_exceeded([](const writable& client) {
        //BUG: Dead connection may stay alive using packet hack.
        client.write(S2C_CLOSE, "b", CC_OVERPOPULATION); 
    });

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

    Socket::GameServer().when(C2S_START,            std::bind(&Player::OnStart,              _1, _2));
    Socket::GameServer().when(C2S_RESTART,          std::bind(&Player::OnRestart,            _1, _2));
    Socket::GameServer().when(C2S_GAMEEXIT,         std::bind(&Player::OnExit,               _1, _2));
    Socket::GameServer().when(C2S_MOVE_ON,          std::bind(&Player::OnMove,               _1, _2, false));
    Socket::GameServer().when(C2S_MOVE_END,         std::bind(&Player::OnMove,               _1, _2, true ));
    Socket::GameServer().when(C2S_CHATTING,         std::bind(&Player::OnChatting,           _1, _2));
    Socket::GameServer().when(C2S_PUTONITEM,        std::bind(&Player::OnPutOnItem,          _1, _2));
    Socket::GameServer().when(C2S_PUTOFFITEM,       std::bind(&Player::OnPutOffItem,         _1, _2));
    Socket::GameServer().when(C2S_USEITEM,          std::bind(&Player::OnUseItem,            _1, _2));
    Socket::GameServer().when(C2S_TRASHITEM,        std::bind(&Player::OnTrashItem,          _1, _2));
    Socket::GameServer().when(C2S_REST,             std::bind(&Player::OnRest,               _1, _2));
    Socket::GameServer().when(C2S_TELEPORT,         std::bind(&Player::OnTeleportAnswer,     _1, _2));
    Socket::GameServer().when(C2S_UPDATEPROPERTY,   std::bind(&Player::OnUpdateProperty,     _1, _2));
    Socket::GameServer().when(C2S_PLAYER_ANIMATION, std::bind(&Player::OnPlayerAnimation,    _1, _2));
    Socket::GameServer().when(C2S_ATTACK,           std::bind(&Player::OnAttack,             _1, _2));
    Socket::GameServer().when(C2S_ASKPARTY,         std::bind(&Player::OnAskParty,           _1, _2));
    Socket::GameServer().when(C2S_ANS_ASKPARTY,     std::bind(&Player::OnAskPartyAnswer,     _1, _2));
    Socket::GameServer().when(C2S_EXILEPARTY,       std::bind(&Player::OnExileParty,         _1, _2));
    Socket::GameServer().when(C2S_LEAVEPARTY,       std::bind(&Player::OnLeaveParty,         _1, _2));
    Socket::GameServer().when(C2S_PICKUPITEM,       std::bind(&Player::OnItemPick,           _1, _2));

    Socket::DBClient().when(D2S_LOGIN,              std::bind(&DBListener::OnLogin,             _1));
    Socket::DBClient().when(D2S_AUTHORIZED,         std::bind(&DBListener::OnAuthorized,        _1));
    Socket::DBClient().when(D2S_SEC_LOGIN,          std::bind(&DBListener::OnSecondaryLogin,    _1));
    Socket::DBClient().when(D2S_PLAYER_INFO,        std::bind(&DBListener::OnPlayerInfo,        _1));
    Socket::DBClient().when(D2S_DELPLAYERINFO,      std::bind(&DBListener::OnDeletePlayerInfo,  _1));
    Socket::DBClient().when(D2S_ANS_NEWPLAYER,      std::bind(&DBListener::OnNewPlayerAnswer,   _1));
    Socket::DBClient().when(D2S_LOADPLAYER,         std::bind(&DBListener::OnLoadPlayer,        _1));
    Socket::DBClient().when(D2S_LOADITEMS,          std::bind(&DBListener::OnLoadItems,         _1));
    Socket::DBClient().when(D2S_UPDATEITEMIID,      std::bind(&DBListener::OnUpdateItemIID,     _1));

    CommandDispatcher::Register("/get",             std::bind(&Player::OnGetItem,           _1, _2));
    CommandDispatcher::Register("/move2",           std::bind(&Player::OnMoveTo,            _1, _2));

    CommandDispatcher::Register("/online", [&](Player& player, CommandDispatcher::Token& token) {
        std::string message = std::string{"Current Online: "} + std::to_string(Socket::GameServer().get_online());
        player.write(packet(S2C_NOTICE, "s", message.c_str()));
    });

    CommandDispatcher::Register("/mob", [&](Player& player, CommandDispatcher::Token& token) {
        int index = token;

        try {
            Monster::Summon(index, player.GetX(), player.GetY(), player.GetMap());
        } catch (const std::exception&) {
            spdlog::warn("Cannot create monster with index {}", index);
        }
    });

    CommandDispatcher::Register("/expelparty", [&](Player& player, CommandDispatcher::Token& token){
        std::string player_name(token);
        World::ForPlayerWithName(player_name, [&](Player& player_to_kick) {
            player.BanFromParty(player_to_kick.GetID());
        });
    });

    CommandDispatcher::Register("/naro", [&](Player& player, CommandDispatcher::Token& token) {
        player.Teleport(258039, 259336);
    });

    CommandDispatcher::Register("/cargo", [&](Player& player, CommandDispatcher::Token& token) {
        player.Teleport(264975, 262548);
    });

    CommandDispatcher::Register("/mine", [&](Player& player, CommandDispatcher::Token& token) {
        player.Teleport(266044, 285024);
    });

    CommandDispatcher::Register("/fort", [&](Player& player, CommandDispatcher::Token& token) {
        player.Teleport(267590, 242885);
    });

    CommandDispatcher::Register("/bird", [&](Player& player, CommandDispatcher::Token& token) {
        player.Teleport(255856, 288742);
    });

    CommandDispatcher::Register("/cop", [&](Player& player, CommandDispatcher::Token& token) {
        player.Teleport(232899, 294628);
    });

    CommandDispatcher::Register("/ghost", [&](Player& player, CommandDispatcher::Token& token) {
        player.Teleport(265500, 238054);
    });

    CommandDispatcher::Register("/tp", [&](Player& player, CommandDispatcher::Token& token) {
        player.Teleport(255680, 229056);
    });

    CommandDispatcher::Register("/getexp", [&](Player& player, CommandDispatcher::Token& token) {
        int exp = token;
        auto lock = player.Lock();
        player.UpdateExp(exp);
    });

    CommandDispatcher::Register("/around", [&](Player& player, CommandDispatcher::Token& token) {
        int radius = token;
        spdlog::info("/around radius: {}; player {}:", radius, player.GetName());

        auto query = WorldMap::QK_PLAYER|WorldMap::QK_MONSTER|WorldMap::QK_NPC;
        World::Map(player.GetMap()).ForEachAround(player, radius, query, [&](Character& character) {
            int distance = player.distance(&character);
            spdlog::info("Character ID: {}; type: {}; distance: {}; coords: ({},{},{})",
                character.GetID(), character.GetType(), distance,
                character.GetX(), character.GetY(), character.GetZ());
        });
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
        {C2S_TELEPORT,          User::INGAME},
        {C2S_UPDATEPROPERTY,    User::INGAME},
        {C2S_PLAYER_ANIMATION,  User::INGAME},
        {C2S_ATTACK,            User::INGAME},
        {C2S_ASKPARTY,          User::INGAME},
        {C2S_ANS_ASKPARTY,      User::INGAME},
        {C2S_LEAVEPARTY,        User::INGAME},
        {C2S_EXILEPARTY,        User::INGAME}
    });

    World::SpawnNpcs();
    World::CreateSpawnsAndSpawnMonsters();

    try 
    {
        Socket::DBClient().connect(db_address, db_port);
        spdlog::info("Connected to DB Server on address {}:{}", db_address, db_port);

        Socket::GameServer().start(game_address, game_port);
        spdlog::info("Game Server has started on address {}:{}", game_address, game_port);
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
                World::Tick();
            }
        } while (status != std::future_status::ready);
    });

    std::cin.get();
    done.set_value();

    World::Cleanup();

    return 0;
}