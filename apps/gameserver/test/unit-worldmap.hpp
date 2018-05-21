typedef std::list<std::pair<const Player*, const bango::space::quad_entity*>> results_t;

static void cleanup(results_t& a1, results_t& a2, results_t& a3)
{
    a1.clear();
    a2.clear();
    a3.clear();
}

TEST(WorldMap, Events)
{
    const int KAL_ZONE_WIDTH=8192;
    const int KAL_ZONE_AMOUNT=50;
    const int KAL_MAX_SIGHT=1024;

    WorldMap map(KAL_ZONE_AMOUNT*KAL_ZONE_WIDTH, KAL_MAX_SIGHT);

    Player p1{1032,1041};
    Player p2{2222,2222};

    results_t appear_results;
    results_t disappear_results;
    results_t move_results;

    map.OnAppear([&](const Player* player, const bango::space::quad_entity* entity) {
        appear_results.push_back(std::make_pair(player, entity));
    });

    map.OnDisappear([&](const Player* player, const bango::space::quad_entity* entity) {
        disappear_results.push_back(std::make_pair(player, entity));
    });

    map.OnMove([&](const Player* player, const bango::space::quad_entity* entity, int new_x, int new_y) {
        move_results.push_back(std::make_pair(player, entity));
    });

    ASSERT_EQ(0, appear_results.size());
    ASSERT_EQ(0, disappear_results.size());
    ASSERT_EQ(0, move_results.size());
    
    map.Add(&p2);

    ASSERT_EQ(0, appear_results.size());
    ASSERT_EQ(0, disappear_results.size());
    ASSERT_EQ(0, move_results.size());

    map.Add(&p1);

    ASSERT_EQ(0, appear_results.size());
    ASSERT_EQ(0, disappear_results.size());
    ASSERT_EQ(0, move_results.size());

    map.Move(&p1, 1432, 1441);

    ASSERT_EQ(0, appear_results.size());
    ASSERT_EQ(0, disappear_results.size());
    ASSERT_EQ(0, move_results.size());

    map.Move(&p1, 1502, 1501);

    ASSERT_EQ(2, appear_results.size());
    ASSERT_EQ(0, disappear_results.size());
    ASSERT_EQ(0, move_results.size());

    ASSERT_EQ(&p2, appear_results.front().first);
    ASSERT_EQ(&p1, appear_results.front().second);
    appear_results.pop_front();
    ASSERT_EQ(&p1, appear_results.front().first);
    ASSERT_EQ(&p2, appear_results.front().second);
    appear_results.pop_front();
    
    map.Move(&p1, 2222, 2222);

    ASSERT_EQ(0, appear_results.size());
    ASSERT_EQ(0, disappear_results.size());
    ASSERT_EQ(1, move_results.size());

    ASSERT_EQ(&p2, move_results.front().first);
    ASSERT_EQ(&p1, move_results.front().second);
    move_results.pop_front();

    map.Move(&p1, 3222, 3222);

    ASSERT_EQ(0, appear_results.size());
    ASSERT_EQ(2, disappear_results.size());
    ASSERT_EQ(0, move_results.size());

    ASSERT_EQ(&p2, disappear_results.front().first);
    ASSERT_EQ(&p1, disappear_results.front().second);
    disappear_results.pop_front();
    ASSERT_EQ(&p1, disappear_results.front().first);
    ASSERT_EQ(&p2, disappear_results.front().second);
    disappear_results.pop_front();
}
