#include "GameManager.h"

void GameManager::Initialize()
{
    m_dbclient.when(D2S_LOGIN, [&](packet& p) {
        m_gameserver.by_id(p.pop<unsigned int>(), [&](const std::shared_ptr<session>& s) {
            s->write(p.change_type(S2C_ANS_LOGIN));
        });
    });

    m_dbclient.when(D2S_AUTHORIZED, [&](packet& p) {
        m_gameserver.by_id(p.pop<unsigned int>(), [&](const std::shared_ptr<session>& s) {
            s->get<Player>()->SetAccountID(p.pop<int>());
        });
    });

    m_dbclient.when(D2S_SEC_LOGIN, [&](packet& p) {
        m_gameserver.by_id(p.pop<unsigned int>(), [&](const std::shared_ptr<session>& s) {
            s->write(p.change_type(S2C_SECOND_LOGIN));
        });
    });

    m_dbclient.when(D2S_PLAYER_INFO, [&](packet& p) {
        m_gameserver.by_id(p.pop<unsigned int>(), [&](const std::shared_ptr<session>& s) {
            s->write(p.change_type(S2C_PLAYERINFO));
        });
    });

    m_dbclient.when(D2S_DELPLAYERINFO, [&](packet& p) {
        m_gameserver.by_id(p.pop<unsigned int>(), [&](const std::shared_ptr<session>& s) {
            s->write(p.change_type(S2C_DELPLAYERINFO));
        });
    });
    
    m_dbclient.when(D2S_ANS_NEWPLAYER, [&](packet& p) {
        m_gameserver.by_id(p.pop<unsigned int>(), [&](const std::shared_ptr<session>& s) {
            s->write(p.change_type(S2C_ANS_NEWPLAYER));
        });
    });

    m_dbclient.when(D2S_LOADPLAYER, [&](packet& p) {
        m_gameserver.by_id(p.pop<unsigned int>(), [&](const std::shared_ptr<session>& s) {
            if (p.pop<char>())
                s->write(S2C_MESSAGE, "b", MSG_NOTEXISTPLAYER);
            else
                s->get<Player>()->OnLoadPlayer(p);
        });
    });

    m_dbclient.when(D2S_LOADITEMS, [&](packet& p) {
        m_gameserver.by_id(p.pop<unsigned int>(), [&](const std::shared_ptr<session>& s) {
            s->get<Player>()->OnLoadItems(p);
        });
    });

    m_gameserver.when(C2S_CONNECT, [&](const std::shared_ptr<session>& s, packet& p) {
        s->write(S2C_CODE, "dbdddIbbb", 0, 0, 604800, 0, 0, 0, 0, 0, 2);
    });

    m_gameserver.when(C2S_ANS_CODE, [&](const std::shared_ptr<session>& s, packet& p) {
        // ignore...
    });

    m_gameserver.when(C2S_LOGIN, [&](const std::shared_ptr<session>& s, packet& p) {
        p.push<unsigned int>(s->get_id());
        m_dbclient.write(p.change_type(S2D_LOGIN));
    });

    m_gameserver.when(C2S_SECOND_LOGIN, [&](const std::shared_ptr<session>& s, packet& p) {
        p.push<int>(s->get<Player>()->GetAccountID());
        p.push<unsigned int>(s->get_id());
        m_dbclient.write(p.change_type(S2D_SECONDARY_LOGIN));
    });

    m_gameserver.when(C2S_NEWPLAYER, [&](const std::shared_ptr<session>& s, packet& p) {
        packet copy = p;
        
        auto name = p.pop_str();
        auto job = p.pop<unsigned char>();
        auto str = p.pop<unsigned short>();
        auto hth = p.pop<unsigned short>();
        auto int_ = p.pop<unsigned short>();
        auto wis = p.pop<unsigned short>();
        auto dex = p.pop<unsigned short>();
        auto face = p.pop<unsigned char>();
        auto hair = p.pop<unsigned char>();

        if (name.empty() || name.size() > 14)
        {
            s->write(S2C_ANS_NEWPLAYER, "b", NA_ERROR);
            return;
        }

        if (job >= CLASS_NUM)
        {
            s->write(S2C_ANS_NEWPLAYER, "b", NA_WRONGCLASS);
            return;
        }

        if (str + hth + int_ + wis + dex != 5)
        {
            s->write(S2C_ANS_NEWPLAYER, "b", NA_WRONGPROPERTY);
            return;
        }

        if (hair > 6 || face > 6)
        {
            s->write(S2C_ANS_NEWPLAYER, "b", NA_WRONGPROPERTY);
            return;
        }

        copy.push<int>(s->get<Player>()->GetAccountID());
        copy.push<unsigned int>(s->get_id());

        m_dbclient.write(copy.change_type(S2D_NEWPLAYER));
    });

    m_gameserver.when(C2S_DELPLAYER, [&](const std::shared_ptr<session>& s, packet& p) {
        p.push<int>(s->get<Player>()->GetAccountID());
        p.push<unsigned int>(s->get_id());

        m_dbclient.write(p.change_type(S2D_DELPLAYER));
    });

    m_gameserver.when(C2S_RESTOREPLAYER, [&](const std::shared_ptr<session>& s, packet& p) {
        p.push<int>(s->get<Player>()->GetAccountID());
        p.push<unsigned int>(s->get_id());

        m_dbclient.write(p.change_type(S2D_RESTOREPLAYER));
    });

    m_gameserver.when(C2S_LOADPLAYER, [&](const std::shared_ptr<session>& s, packet& p) {
        p.push<int>(s->get<Player>()->GetAccountID());
        p.push<unsigned int>(s->get_id());

        m_dbclient.write(p.change_type(S2D_LOADPLAYER));
    });

    m_gameserver.when(C2S_START, [&](const std::shared_ptr<session>& s, packet& p) {
        // TODO: Add check if already in game
        s->get<Player>()->GameStart(p);
    });

    m_gameserver.when(C2S_RESTART, [&](const std::shared_ptr<session>& s, packet& p) {
        // TODO: Check if ingame
        if (p.pop<char>() == 1) // Can I logout?
            s->write(S2C_ANS_RESTART, "b", s->get<Player>()->CanLogout() ? 1 : 0); // 1=Yes, 0=No -> In Fight? PVP? Etc
        else
        {
            s->get<Player>()->GameRestart();
            m_dbclient.write(S2D_RESTART, "dd", s->get_id(), s->get<Player>()->GetAccountID());
        }
    });

    m_gameserver.on_connected([&](const std::shared_ptr<session>& s) {
        std::cout << "::> " << s->get_host() << " has connected" << std::endl;
    });

    m_gameserver.on_disconnected([&](const std::shared_ptr<session>& s) {
        m_dbclient.write(S2D_DISCONNECT, "d", s->get<Player>()->GetAccountID());
        std::cout << "::< " << s->get_host() << " has disconnected" << std::endl; 
    });
}

bool GameManager::ConnectToDatabase(const std::string& host, const std::int32_t port)
{
    return m_dbclient.connect(host, port);
}

bool GameManager::StartGameServer(const std::string& host, const std::int32_t port)
{
    return m_gameserver.start(host, port);
}