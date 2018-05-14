

TEST(MapTest, AppearDisappear)
{
    map m(4, 3, 1024, 512);

    notifable_entity player{1022,1023,"Player"};
    std::string event;
    static_entity e1{1544,1553,"FarAway"};
    static_entity e2{1032,1041,"CloseObject"};
    dynamic_entity e3{1032,1041,"Dynamic"};
    notifable_entity e4{1032,1041,"Notifable"};

    m.on_appear([&](notifable_entity* ne, entity* e) {
        event = "appear:" + ne->m_name + e->m_name;
    });

    m.on_disappear([&](notifable_entity* ne, entity* e) {
        event = "disappear:" + ne->m_name + e->m_name;
    });

    m.insert(&player);
    EXPECT_EQ("", event);

    m.insert(&e1);
    EXPECT_EQ("", event);

    m.insert(&e2);
    EXPECT_EQ("appear:PlayerCloseObject", event);
    event = "";

    m.remove(&e1);
    EXPECT_EQ("", event);
    
    m.remove(&e2);
    EXPECT_EQ("disappear:PlayerCloseObject", event);
    event = "";

    m.insert(&e3);
    EXPECT_EQ("appear:PlayerDynamic", event);
    event = "";

    m.insert(&e4);
    EXPECT_EQ("appear:NotifableDynamic", event);
    event = "";

    m.remove(&e3);
    EXPECT_EQ("disappear:NotifableDynamic", event);
    event = "";

    m.remove(&e4);
    EXPECT_EQ("disappear:PlayerNotifable", event);
    event = "";
}

TEST(MapTest, Move)
{
    map m(4, 3, 1024, 512);

    dynamic_entity player{1000,1000,"John"};

    EXPECT_EQ(0, m.at(0, 0)->size());

    m.insert(&player);
    EXPECT_EQ(1, m.at(0, 0)->size());

    m.move(&player, 1032, 1005);
    EXPECT_EQ(1, m.at(1, 0)->size());

    m.move(&player, 1033, 1006);
    EXPECT_EQ(1, m.at(1, 0)->size());
    
    m.move(&player, 1034, 1025);
    EXPECT_EQ(1, m.at(1, 1)->size());

    m.remove(&player);
    EXPECT_EQ(0, m.at(1, 1)->size());
}

class appear : public std::exception {
    virtual const char* what() const throw() {
        return "appear";
    }
};

class disappear : public std::exception {
    virtual const char* what() const throw() {
        return "disappear";
    }
};

class move : public std::exception {
    virtual const char* what() const throw() {
        return "move";
    }
};

TEST(MapTest, MoveNoEvents)
{
    map m(4, 3, 1024, 512);

    static_entity npc{1001,1002,"Priest"};
    dynamic_entity mob{1000,1000,"Vulgar"};

    m.on_appear([&](notifable_entity* ne, entity* e) {
        throw new appear;
    });

    m.on_disappear([&](notifable_entity* ne, entity* e) {
        throw new disappear;
    });

    m.on_move([&](notifable_entity* ne, entity* e, int new_x, int new_y) {
        throw new move;
    });

    EXPECT_NO_THROW({m.insert(&npc);});
    EXPECT_NO_THROW({m.insert(&mob);});
    EXPECT_NO_THROW({m.move(&mob, 1022, 1023);});
    EXPECT_NO_THROW({m.remove(&npc);});
    EXPECT_NO_THROW({m.remove(&mob);});
}

TEST(MapTest, MoveEvents)
{
    map m(4, 3, 1024, 512);

    static_entity npc{1001,1002,"Priest"};
    dynamic_entity mob{1000,1000,"Vulgar"};
    notifable_entity player{600,600,"Thief"};

    std::list<std::string> events;

    m.on_appear([&](notifable_entity* ne, entity* e) {
        events.push_back("appear:" + ne->m_name + e->m_name);
    });

    m.on_disappear([&](notifable_entity* ne, entity* e) {
        events.push_back("disappear:" + ne->m_name + e->m_name);
    });

    m.on_move([&](notifable_entity* ne, entity* e, int new_x, int new_y) {
        events.push_back("move:" + ne->m_name + e->m_name);
    });

    // NPC spawned.
    m.insert(&npc);
    ASSERT_EQ(0, events.size());

    // Monster spawned next to NPC.
    m.insert(&mob);
    ASSERT_EQ(0, events.size());

    // Player spawned far away from NPC & Monster.
    m.insert(&player);
    ASSERT_EQ(0, events.size());

    // Monster makes small step.
    m.move(&mob, 1022, 1023);
    ASSERT_EQ(0, events.size());

    // Player makes small step.
    m.move(&player, 602, 602);
    ASSERT_EQ(0, events.size());

    // Player makes bigger step and reaches NPC.
    m.move(&player, 652, 652);
    ASSERT_EQ(1, events.size());
    ASSERT_EQ("appear:ThiefPriest", events.front());
    events.pop_front();

    // Player comes close to both NPC & Monster.
    m.move(&player, 1025, 1004);
    ASSERT_EQ(1, events.size());
    ASSERT_EQ("appear:ThiefVulgar", events.front());
    events.pop_front();

    // Player moves out of NPC sight
    m.move(&player, 1367, 1367);
    ASSERT_EQ(1, events.size());
    ASSERT_EQ("disappear:ThiefPriest", events.front());
    events.pop_front();

    // Player goes back.
    m.move(&player, 1025, 1004);
    ASSERT_EQ(1, events.size());
    ASSERT_EQ("appear:ThiefPriest", events.front());
    events.pop_front();

    // Player logs out.
    m.remove(&player);
    ASSERT_EQ(0, events.size());
}

TEST(MapTest, MoveManyNotifables)
{
    map m(4, 3, 1024, 512);

    notifable_entity thief{600,600,"Thief"};
    notifable_entity shaman{100,100,"Shaman"};

    std::list<std::string> events;

    m.on_appear([&](notifable_entity* ne, entity* e) {
        events.push_back("appear:" + ne->m_name + e->m_name);
    });

    m.on_disappear([&](notifable_entity* ne, entity* e) {
        events.push_back("disappear:" + ne->m_name + e->m_name);
    });

    m.on_move([&](notifable_entity* ne, entity* e, int new_x, int new_y) {
        events.push_back("move:" + ne->m_name + e->m_name);
    });

    // Both players spawn far away of each other
    m.insert(&thief);
    m.insert(&shaman);
    ASSERT_EQ(0, events.size());

    // Shaman comes close to Thief
    m.move(&shaman, 599, 599);
    ASSERT_EQ(2, events.size());
    ASSERT_EQ("appear:ShamanThief", events.front());
    events.pop_front();
    ASSERT_EQ("appear:ThiefShaman", events.front());
    events.pop_front();

    // Thief moves a bit
    m.move(&thief, 601, 601);
    ASSERT_EQ(1, events.size());
    ASSERT_EQ("move:ShamanThief", events.front());
    events.pop_front();

    // Thief goes out of sight
    m.move(&thief, 1028, 1022);
    ASSERT_EQ(2, events.size());
    ASSERT_EQ("disappear:ThiefShaman", events.front());
    events.pop_front();
    ASSERT_EQ("disappear:ShamanThief", events.front());
    events.pop_front();

    // Thief comes back
    m.move(&thief, 599, 599);
    ASSERT_EQ(2, events.size());
    ASSERT_EQ("appear:ThiefShaman", events.front());
    events.pop_front();
    ASSERT_EQ("appear:ShamanThief", events.front());
    events.pop_front();

    // Shaman logs out
    m.remove(&shaman);
    ASSERT_EQ(1, events.size());
    ASSERT_EQ("disappear:ThiefShaman", events.front());
    events.pop_front();

    // Thief logs out
    m.remove(&thief);
    ASSERT_EQ(0, events.size());
}

TEST(MapTest, MassiveMove)
{
    map m(4, 3, 1024, 512);

    std::list<static_entity*> s;
    std::list<dynamic_entity*> d;
    std::list<notifable_entity*> n;
    std::list<std::string> events;

    m.on_appear([&](notifable_entity* ne, entity* e) {
        events.push_back("appear:" + ne->m_name + e->m_name);
    });

    m.on_disappear([&](notifable_entity* ne, entity* e) {
        events.push_back("disappear:" + ne->m_name + e->m_name);
    });

    m.on_move([&](notifable_entity* ne, entity* e, int new_x, int new_y) {
        events.push_back("move:" + ne->m_name + e->m_name);
    });

    for (int i = 0; i < 100; i++)
        s.push_back(new static_entity(30, 30, ""));
    for (int i = 0; i < 100; i++)
        d.push_back(new dynamic_entity(30, 30, ""));
    for (int i = 0; i < 100; i++)
        n.push_back(new notifable_entity(30, 30, ""));

    ASSERT_EQ(0, events.size());

    for (auto& e : s)
        m.insert(e);
    ASSERT_EQ(0, events.size());

    for (auto& e : d)
        m.insert(e);
    ASSERT_EQ(0, events.size());

    for (auto& e : n)
        m.insert(e);
    ASSERT_EQ(29900, events.size());
    // 200
    // 201 + 1
    // 202 + 2
    // 203 + 3
    // ...
    // 299 + 99

    events.clear();

    // Move Monster
    m.move(d.front(), 30, 31);
    ASSERT_EQ(100, events.size());

    events.clear();

    // Move Player
    m.move(n.front(), 30, 31);
    ASSERT_EQ(99, events.size());
    // 100-1 the one who moved

    for (auto& e : s)
        delete e;
    for (auto& e : d)
        delete e;
    for (auto& e : n)
        delete e;
}