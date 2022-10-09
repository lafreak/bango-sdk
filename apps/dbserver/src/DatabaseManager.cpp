#include "DatabaseManager.h"

void DatabaseManager::Initialize()
{
    m_dbserver.when(S2D_DISCONNECT, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        FlagDisconnected(p.pop<int>());
    });

    m_dbserver.when(S2D_LOGIN, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        Login(s, p);
    });

    m_dbserver.when(S2D_SECONDARY_LOGIN, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        switch (p.pop<char>())
        {
            case SL_LOGIN:              SecondaryLogin (s, p); break;
            case SL_CREATE_PASSWORD:    SecondaryCreate(s, p); break;
            case SL_CHANGE_PASSWORD:    SecondaryChange(s, p); break;
        }
    });

    m_dbserver.when(S2D_NEWPLAYER, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        NewPlayer(s, p);
    });

    m_dbserver.when(S2D_DELPLAYER, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        DeletePlayer(s, p);
    });

    m_dbserver.when(S2D_RESTOREPLAYER, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        RestorePlayer(s, p);
    });

    m_dbserver.when(S2D_LOADPLAYER, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        LoadPlayer(s, p);
    });
    
    m_dbserver.when(S2D_INSERTITEM, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        InsertItem(s, p);
    });

    m_dbserver.when(S2D_UPDATEITEMNUM, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        UpdateItemNum(s, p);
    });

    m_dbserver.when(S2D_UPDATEITEMINFO, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        UpdateItemInfo(s, p);
    });

    m_dbserver.when(S2D_TRASHITEM, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        TrashItem(s, p);
    });

    m_dbserver.when(S2D_UPDATEPROPERTY, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        UpdateProperty(s, p);
    });
    
    m_dbserver.when(S2D_RESTART, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        auto id = p.pop<unsigned int>();
        auto idaccount = p.pop<int>();
        SendPlayerList(s, id, idaccount);
    });

    m_dbserver.when(S2D_SAVEALLPROPERTY, [&](const std::shared_ptr<GameServer>& s, packet& p) {
        SaveAllProperty(s, p);
    });

    m_dbserver.on_disconnected([&](const std::shared_ptr<GameServer>& s) {
        m_active_users.clear();
    });
}

void DatabaseManager::ConnectToPool(const std::string& host, const std::string& port, const std::string& user, const std::string& password, const std::string& schema)
{
    m_pool.connect(host, port, user, password, schema);
}

void DatabaseManager::StartDBServer(const std::string& host, const std::int32_t port)
{
    m_dbserver.start(host, port);
}

bool DatabaseManager::Validate(const std::string& password) const
{
    if (password.size() != 8)
        return false;

    for (auto c : password)
    {
        if (c < '0' || c > '9')
            return false;
    }

    return true;
}

void DatabaseManager::Login(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto login = p.pop_str();
    auto passwd = p.pop_str();
    auto mac = p.pop_str();
    auto id = p.pop<unsigned int>();

    auto conn = m_pool.get();
    auto query = conn.create_query(
        "SELECT "
        "EXISTS(SELECT 1 FROM account WHERE login=?), "
        "EXISTS(SELECT 1 FROM account WHERE login=? AND password=?), "
        "(SELECT idaccount FROM account WHERE login=? AND password=?), "
        "(SELECT secondary FROM account WHERE login=? AND password=?)");

    //query.set(login).set(login).set(passwd).set(login).set(passwd).set(login).set(passwd);
    query << login << login << passwd << login << passwd << login << passwd;

    query.execute_query();
    query.next();

    auto correct_login = query.get_int();
    auto correct_password = query.get_int();

    if (!correct_login)
    {
        s->write(D2S_LOGIN, "db", id, LA_WRONGID);
        return;
    }

    if (!correct_password)
    {
        s->write(D2S_LOGIN, "db", id, LA_WRONGPWD);
        return;
    }

    auto idaccount = query.get_int();
    if (!m_active_users.insert(idaccount).second)
    {
        s->write(D2S_LOGIN, "db", id, LA_SAMEUSER);
        return;
    }

    s->write(D2S_AUTHORIZED, "dd", id, idaccount);

    auto secondary = query.get_str();
    if (secondary.empty())
    {
        s->write(D2S_LOGIN, "db", id, LA_CREATE_SECONDARY);
        return;
    }
    
#ifndef DISABLE_SECONDARY_PASSWORD
        s->write(D2S_LOGIN, "db", id, LA_OK);
#else
        SendPlayerList(s, id, idaccount);
#endif
}

void DatabaseManager::SecondaryLogin(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto password = p.pop_str();
    if (!Validate(password))
        return;

    p.pop<int>(); //?

    auto idaccount = p.pop<int>();
    auto id = p.pop<unsigned int>();

    auto conn = m_pool.get();
    auto query = conn.create_query(
        "SELECT secondary FROM account WHERE idaccount=?");

    query.set(idaccount);
    query.execute_query();
    query.next();

    auto result = query.get_str();
    
    if (password != result)
    {
        s->write(D2S_SEC_LOGIN, "dbb", id, SL_RESULT_MSG, MSL_WRONG_PWD);
        return;
    }

    SendPlayerList(s, id, idaccount);
}

void DatabaseManager::SecondaryCreate(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto password = p.pop_str();
    auto secondary = p.pop_str();
    auto idaccount = p.pop<int>();
    auto id = p.pop<unsigned int>();

    if (!Validate(secondary))
        return;

    auto conn = m_pool.get();
    auto query = conn.create_query(
        "UPDATE account SET secondary=? WHERE idaccount=? AND password=?");

    query.set(secondary).set(idaccount).set(password);
    if (query.execute() == 0)
    {
        s->write(D2S_SEC_LOGIN, "dbb", id, SL_RESULT_MSG, MSL_WRONG_PWD);
        return;
    }

    s->write(D2S_LOGIN, "dbd", id, LA_OK, idaccount);
}

void DatabaseManager::SecondaryChange(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto old_pass = p.pop_str();
    auto new_pass = p.pop_str();
    auto idaccount = p.pop<int>();
    auto id = p.pop<unsigned int>();

    if (!Validate(new_pass))
        return;

    auto conn = m_pool.get();
    auto query = conn.create_query(
        "UPDATE account SET secondary=? WHERE idaccount=? AND secondary=?");

    query.set(new_pass).set(idaccount).set(old_pass);
    if (query.execute() == 0)
    {
        s->write(D2S_SEC_LOGIN, "dbb", id, SL_RESULT_MSG, MSL_WRONG_PWD);
        return;
    }

    s->write(D2S_LOGIN, "dbd", id, LA_OK, idaccount);
}

void DatabaseManager::SendPlayerList(const std::shared_ptr<GameServer>& s, unsigned int id, int idaccount)
{
    auto conn = m_pool.get();
    auto query = conn.create_query(
        "SELECT * FROM player WHERE idaccount=? AND deleted=0 ORDER BY level DESC");

    query.set(idaccount);
    query.execute_query();

    packet p(D2S_PLAYER_INFO);

    p.push<unsigned int>(id);

    p.push<char>(0);// Auth
    p.push<char>(0);// Unknown
    p.push<int>(0);// ExpTime

    packet players;
    unsigned char player_count = 0;
    while (query.next())
    {
        player_count++;

        auto idplayer = query.get_int("idplayer");

        players.push<int>(idplayer);
        players.push_str(query.get_str("name"));
        players.push<unsigned char>(query.get_int("class"));
        players.push<unsigned char>(query.get_int("job"));
        players.push<unsigned char>(query.get_int("level"));
        players.push<int>(0); // GID
        players.push<unsigned short>(query.get_int("strength"));
        players.push<unsigned short>(query.get_int("health"));
        players.push<unsigned short>(query.get_int("inteligence"));
        players.push<unsigned short>(query.get_int("wisdom"));
        players.push<unsigned short>(query.get_int("dexterity"));
        players.push<unsigned char>(query.get_int("face"));
        players.push<unsigned char>(query.get_int("hair"));

        auto subquery = conn.create_query(
            "SELECT `index` FROM item WHERE idplayer=? AND (info & 1)");

        subquery.set(idplayer);
        subquery.execute_query();

        packet items;
        unsigned char item_count = 0;
        while (subquery.next())
        {
            item_count++;
            items.push<unsigned short>(subquery.get_int("index"));
        }

        players.push<char>(item_count);
        players << items;
    }

    p.push<char>(player_count);
    p << players;

    s->write(p);
    SendDeletedList(s, id, idaccount);
}

void DatabaseManager::SendDeletedList(const std::shared_ptr<GameServer>& s, unsigned int id, int idaccount)
{
    auto conn = m_pool.get();
    auto query = conn.create_query("SELECT * FROM player WHERE idaccount=? AND deleted=1");

    query.set(idaccount);
    query.execute_query();

    packet p(D2S_DELPLAYERINFO);
    packet players;
    unsigned char player_count = 0;

    while (query.next())
    {
        player_count++;

        players.push<int>(query.get_int("idplayer"));
        players.push_str(query.get_str("name"));
        players.push<char>(query.get_int("level"));
        players.push<char>(query.get_int("job"));
        players.push<unsigned char>(255); // remaining_days
    }

    p.push<unsigned int>(id);
    p.push<char>(player_count);
    p << players;
    s->write(p);
}

void DatabaseManager::NewPlayer(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto name =     p.pop_str();
    auto job =      p.pop<unsigned char>();
    auto str =      p.pop<unsigned short>();
    auto hth =      p.pop<unsigned short>();
    auto int_ =     p.pop<unsigned short>();
    auto wis =      p.pop<unsigned short>();
    auto dex =      p.pop<unsigned short>();
    auto face =     p.pop<unsigned char>();
    auto hair =     p.pop<unsigned char>();

    auto idaccount = p.pop<int>();
    auto id = p.pop<unsigned int>();

    auto conn = m_pool.get();
    auto query = conn.create_query("SELECT "
        "EXISTS(SELECT 1 FROM player WHERE name=?), "
        "(SELECT COUNT(*) FROM player WHERE idaccount=? AND deleted=0)");

    query.set(name).set(idaccount);
    query.execute_query();
    query.next();

    auto duplicate = query.get_int();
    auto count = query.get_int();

    if (duplicate)
    {
        s->write(D2S_ANS_NEWPLAYER, "db", id, NA_OCCUPIEDID);
        return;
    }

    if (count >= MAX_CHARACTER)
    {
        s->write(D2S_ANS_NEWPLAYER, "db", id, NA_OVERPLAYERNUM);
        return;
    }

    auto insert = conn.create_query("INSERT INTO player "
        "(idaccount, name, class, strength, health, inteligence, wisdom, dexterity, curhp, curmp, face, hair) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?)");

    str += g_baseproperty[job].prty[P_STR];
    hth += g_baseproperty[job].prty[P_HTH];
    int_+= g_baseproperty[job].prty[P_INT];
    wis += g_baseproperty[job].prty[P_WIS];
    dex += g_baseproperty[job].prty[P_DEX];

    auto curhp = g_denoHP[job] * hth;
    auto curmp = g_denoMP[job] * wis;

    insert << idaccount << name << job;;
    insert << str << hth << int_ << wis << dex;
    insert << curhp << curmp;
    insert << face << hair;

    if (insert.execute() != 1)
    {
        std::cerr << __FUNCTION__ << ": Error" << std::endl;
        return;
    }

    SendPlayerList(s, id, idaccount);
}

void DatabaseManager::DeletePlayer(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto idplayer = p.pop<int>();
    auto idaccount = p.pop<int>();
    auto id = p.pop<unsigned int>();

    auto conn = m_pool.get();
    auto query = conn.create_query(
        "UPDATE player SET deleted=1 WHERE idaccount=? AND idplayer=? AND deleted=0");

    query << idaccount << idplayer;
    
    if (query.execute() == 0)
    {
        std::cerr << __FUNCTION__ << ": Packet manipulation" << std::endl;
        return;
    }

    SendPlayerList(s, id, idaccount);
}

void DatabaseManager::RestorePlayer(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto idplayer = p.pop<int>();
    auto idaccount = p.pop<int>();
    auto id = p.pop<unsigned int>();

    auto conn = m_pool.get();
    auto query = conn.create_query(
        "UPDATE player SET deleted=0 WHERE idaccount=? AND idplayer=? AND deleted=1");

    query << idaccount << idplayer;
    
    if (query.execute() == 0)
    {
        std::cerr << __FUNCTION__ << ": Packet manipulation" << std::endl;
        return;
    }

    SendPlayerList(s, id, idaccount);
}

void DatabaseManager::LoadPlayer(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto idplayer = p.pop<int>();
    auto idguild = p.pop<int>(); //0
    auto show_honor_grade = p.pop<int>(); //?

    auto idaccount = p.pop<int>();
    auto id = p.pop<unsigned int>();

    auto conn = m_pool.get();
    auto query = conn.create_query(
        "SELECT * FROM player WHERE idplayer=? AND idaccount=? AND deleted=0");

    query << idplayer << idaccount;
    query.execute_query();

    if (!query.next())
    {
        s->write(D2S_LOADPLAYER, "db", id, 1); // 1=Player Doesnt Exist
        return;
    }

    packet out(D2S_LOADPLAYER);
    
    out.push<unsigned int>(id);
    out.push<char>(0); //0=Player Exists All cool
    out.push<int>(idaccount);
    out.push<int>(idplayer);
    out.push<char>(query.get_int("class"));
    out.push<char>(query.get_int("job"));
    out.push<char>(query.get_int("level"));
    out.push<char>(query.get_int("map"));
    out.push<unsigned short>(query.get_int("strength"));
    out.push<unsigned short>(query.get_int("health"));
    out.push<unsigned short>(query.get_int("inteligence"));
    out.push<unsigned short>(query.get_int("wisdom"));
    out.push<unsigned short>(query.get_int("dexterity"));
    out.push<unsigned int>(query.get_int("curhp"));
    out.push<unsigned int>(query.get_int("curmp"));
    out.push<unsigned long>(query.get_long("exp"));
    out.push<unsigned short>(query.get_int("pupoint"));
    out.push<unsigned short>(query.get_int("supoint"));
    out.push<unsigned short>(query.get_int("contribute"));
    out.push<unsigned int>(query.get_int("anger"));
    out.push<int>(query.get_int("z"));
    out.push<char>(query.get_int("face"));
    out.push<char>(query.get_int("hair"));
    out.push_str(query.get_str("name"));
    out.push<int>(query.get_int("x"));
    out.push<int>(query.get_int("y"));

    s->write(out);

    LoadItems(s, id, idplayer);
}

void DatabaseManager::LoadItems(const std::shared_ptr<GameServer>& s, unsigned int id, int idplayer)
{
    auto conn = m_pool.get();
    auto query = conn.create_query("SELECT * FROM item WHERE idplayer=?");

    query << idplayer;
    query.execute_query();
    
    packet items;
    unsigned short item_count=0;
    while (query.next())
    {
        item_count++;

        ITEMINFO info={};
        
        info.IID        = query.get_int("iditem");
        info.Index      = query.get_int("index");
        info.Num        = query.get_int("num");
        info.Info       = query.get_int("info");
        info.Prefix     = query.get_int("prefix");
        info.CurEnd     = query.get_int("curend");
        //info.MaxEnd     = 0;//query.get_int("maxend"); // TODO: Move to InitItem
        info.XAttack    = query.get_int("xattack");
        info.XMagic     = query.get_int("xmagic");
        info.XDefense   = query.get_int("xdefense");
        info.XHit       = query.get_int("xhit");
        info.XDodge     = query.get_int("xdodge");
        info.WeaponLevel        = query.get_int("explosiveblow");
        info.FuseInfo.Level     = query.get_int("fusion");
        info.FuseInfo.Meele     = query.get_int("fmeele");
        info.FuseInfo.Magic     = query.get_int("fmagic");
        info.FuseInfo.Defense   = query.get_int("fdefense");
        info.FuseInfo.Absorb    = query.get_int("fabsorb");
        info.FuseInfo.Dodge     = query.get_int("fevasion");
        info.FuseInfo.Hit       = query.get_int("fhit");
        info.FuseInfo.HP        = query.get_int("fhp");
        info.FuseInfo.MP        = query.get_int("fmp");
        info.FuseInfo.Stat[P_STR]       = query.get_int("fstr");
        info.FuseInfo.Stat[P_HTH]       = query.get_int("fhth");
        info.FuseInfo.Stat[P_INT]       = query.get_int("fint");
        info.FuseInfo.Stat[P_WIS]       = query.get_int("fwis");
        info.FuseInfo.Stat[P_DEX]       = query.get_int("fdex");
        info.Shot               = query.get_int("shot");
        info.Perforation        = query.get_int("perforation");
        info.GongA              = query.get_int("gongleft");
        info.GongB              = query.get_int("gongright");

        items.push<ITEMINFO>(info);
    }

    packet p(D2S_LOADITEMS);
    p.push<unsigned int>(id);
    p.push<unsigned short>(item_count);
    p << items;

    s->write(p);
}

void DatabaseManager::InsertItem(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto info = p.pop<ITEMINFO>();
    auto idplayer = p.pop<int>();
    auto id = p.pop<unsigned int>();
    auto local_id = p.pop<unsigned int>();

    auto conn = m_pool.get();
    auto query = conn.create_query("INSERT INTO item "
        "(idplayer, `index`, num, info, prefix, curend, xattack, xmagic, xdefense, xhit, xdodge, "
        "explosiveblow, fusion, fmeele, fmagic, fdefense, fabsorb, fevasion, fhit, fhp, fmp, "
        "fstr, fhth, fint, fwis, fdex, shot, perforation, gongleft, gongright) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");

    query   << idplayer
            << info.Index
            << info.Num
            << info.Info
            << info.Prefix
            << info.CurEnd
            << info.XAttack
            << info.XMagic
            << info.XDefense
            << info.XHit
            << info.XDodge
            << info.WeaponLevel
            << info.FuseInfo.Level
            << info.FuseInfo.Meele
            << info.FuseInfo.Magic
            << info.FuseInfo.Defense
            << info.FuseInfo.Absorb
            << info.FuseInfo.Dodge
            << info.FuseInfo.Hit
            << info.FuseInfo.HP
            << info.FuseInfo.MP
            << info.FuseInfo.Stat[P_STR]
            << info.FuseInfo.Stat[P_HTH]
            << info.FuseInfo.Stat[P_INT]
            << info.FuseInfo.Stat[P_WIS]
            << info.FuseInfo.Stat[P_DEX]
            << info.Shot
            << info.Perforation
            << info.GongA
            << info.GongB;

    query.execute();

    auto res = conn.create_query("SELECT LAST_INSERT_ID()");
    res.execute_query();
    res.next();

    auto iid = res.get_int();
    s->write(D2S_UPDATEITEMIID, "ddd", id, local_id, iid);
}

void DatabaseManager::UpdateItemNum(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto iid = p.pop<int>();
    auto num = p.pop<unsigned int>();

    auto conn = m_pool.get();
    auto query = conn.create_query("UPDATE item SET num=? WHERE iditem=?");

    query << num << iid;
    query.execute();
}

void DatabaseManager::TrashItem(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto conn = m_pool.get();
    auto query = conn.create_query("DELETE FROM item WHERE iditem=?");

    query << p.pop<int>();
    query.execute();
}

void DatabaseManager::UpdateItemInfo(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto iid = p.pop<int>();
    auto info = p.pop<unsigned int>();

    auto conn = m_pool.get();
    auto query = conn.create_query("UPDATE item SET info=? WHERE iditem=?");

    query << info << iid;
    query.execute();
}

void DatabaseManager::UpdateProperty(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto pid = p.pop<int>();

    std::uint16_t stats[5] = {
        p.pop<std::uint16_t>(),
        p.pop<std::uint16_t>(),
        p.pop<std::uint16_t>(),
        p.pop<std::uint16_t>(),
        p.pop<std::uint16_t>()
    };

    auto pupoint = p.pop<std::uint16_t>();

    auto conn = m_pool.get();
    auto query = conn.create_query("UPDATE player SET strength=?, health=?, inteligence=?, wisdom=?, dexterity=?, pupoint=? WHERE idplayer=?");

    query << stats[P_STR] << stats[P_HTH] << stats[P_INT] << stats[P_WIS] << stats[P_DEX] << pupoint << pid;
    query.execute();
}

void DatabaseManager::SaveAllProperty(const std::shared_ptr<GameServer>& s, packet& p)
{
    auto pid = p.pop<int>();
    auto level = p.pop<unsigned char>();
    auto x = p.pop<int>();
    auto y = p.pop<int>();
    auto z = p.pop<int>();
    auto contribute = p.pop<unsigned short>();
    auto cur_hp = p.pop<unsigned int>();
    auto cur_mp = p.pop<unsigned int>();
    auto exp = p.pop<unsigned long>();
    auto pupoint = p.pop<unsigned short>();
    auto supoint = p.pop<unsigned short>();
    auto rage = p.pop<unsigned int>();


    auto conn = m_pool.get();
    auto query = conn.create_query("UPDATE player SET level=?, x=?, y=?, z=?, contribute=?, curhp=?, curmp=?, exp=?, pupoint=?, supoint=?, anger=? WHERE idplayer=?");

    query << level << x << y << z << contribute << cur_hp << cur_mp << exp << pupoint << supoint << rage << pid;
    query.execute();
}