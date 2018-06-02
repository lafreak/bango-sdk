class my_container : public quad_entity_container<my_container>
{
    std::list<const quad_entity*> m_entities;

public:
    void insert(const quad_entity* entity) override
    {
        m_entities.push_back(entity);
    }
    void remove(const quad_entity* entity) override
    {
        m_entities.remove(entity);
    }
    void merge(const my_container* container) override
    {
        m_entities.insert(m_entities.end(), container->m_entities.begin(), container->m_entities.end());
    }
    size_t size() const override
    {
        return m_entities.size();
    }
    long long total_memory() const override
    {
        return sizeof(m_entities)+m_entities.size()*sizeof(const quad_entity*);
    }
    void for_each(const std::function<void(const quad_entity*)>&& callback) const override
    {
        for (auto& qe : m_entities)
            callback(qe);
    }
};

static void BM_QuadTree_Query(benchmark::State &state)
{
    quad<my_container> q(square{
        {0,0}, 
        128
    });

    std::vector<quad_entity> entities;

    for (int i = 0; i < 128; i+=state.range(0))
    {
        for (int j = 0; j < 128; j+=state.range(0))
        {
            entities.push_back(quad_entity{i,j});
        }
    }

    for (auto& qe : entities)
        q.insert(&qe);

    int distance=5;
    point center{11,24};

    const quad_entity* k=nullptr;

    for (auto _ : state)
    {
        q.query(center, distance, [&](const my_container* container) {
            container->for_each([&](const quad_entity* e) {
                if (e->distance(center) < distance)
                    benchmark::DoNotOptimize(k=e);
            });
        });
    }
}

static void BM_QuadTree_Move(benchmark::State &state)
{
    quad<my_container> q(square{
        {0,0}, 
        128
    });

    std::vector<quad_entity> entities;

    for (int i = 0; i < 128; i+=state.range(0))
        for (int j = 0; j < 128; j+=state.range(0))
            entities.push_back(quad_entity{i,j});

    for (auto& qe : entities)
        q.insert(&qe);

    quad_entity actor{0,0};
    q.insert(&actor);

    for (auto _ : state)
    {
        q.remove(&actor);

        if (actor.m_x==127)
            actor.m_y = (actor.m_y+1)%128;
        actor.m_x = (actor.m_x+1)%128;

        q.insert(&actor);
    }
}

static void BM_QuadTree_QueryWithMaxEntityAmount(benchmark::State &state)
{
    quad<my_container> q(square{
        {0,0}, 
        128
    }, state.range(0));

    std::vector<quad_entity> entities;

    for (int i = 0; i < 128; i++)
        for (int j = 0; j < 128; j++)
            entities.push_back(quad_entity{i,j});

    for (auto& qe : entities)
        q.insert(&qe);

    int distance=5;
    point center{11,24};

    const quad_entity* k=nullptr;

    for (auto _ : state)
    {
        q.query(center, distance, [&](const my_container* container) {
            container->for_each([&](const quad_entity* e) {
                if (e->distance(center) < distance)
                    benchmark::DoNotOptimize(k=e);
            });
        });
    }
}

static void BM_QuadTreeTraverse(benchmark::State &state)
{
    quad<my_container> q(square{
        {0,0}, 
        128
    }, 32);

    std::vector<quad_entity> entities;

    for (int i = 0; i < 128; i++)
        for (int j = 0; j < 128; j++)
            entities.push_back(quad_entity{i,j});

    for (auto& qe : entities)
        q.insert(&qe);

    for (auto _ : state)
    {
        int i = 0;
        q.query(point{64,64}, state.range(0), [&](const my_container* container) {
            container->for_each([&](const quad_entity* e) {
                i += 1;
            });
        });

        assert(i == 128*128);
    }
}

static void BM_MapTraverse(benchmark::State &state)
{
    std::map<unsigned int, const quad_entity*> map;

    std::vector<quad_entity> entities;

    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            entities.push_back(quad_entity{i,j});
            map.insert(std::make_pair((unsigned int)i*128+j, &entities[i*128+j]));
        }
    }

    for (auto _ : state)
    {
        int i = 0;

        for (const auto& pair : map)
            i++;

        assert(i == 128*128);
    }
}

static void BM_QuadFind(benchmark::State &state)
{
    quad<my_container> q(square{
        {0,0}, 
        128
    }, 32);

    std::vector<quad_entity> entities;

    for (int i = 0; i < 128; i++)
        for (int j = 0; j < 128; j++)
            entities.push_back(quad_entity{i,j});

    for (auto& qe : entities)
        q.insert(&qe);

    for (auto _ : state)
    {
        int i = 0;
        q.query(point{64,64}, state.range(0), [&](const my_container* container) {
            container->for_each([&](const quad_entity* e) {
                if (e->m_x == 63 && e->m_y == 63) i++;
            });
        });

        benchmark::DoNotOptimize(i);
    }
}

static void BM_QuadFindByCoord(benchmark::State &state)
{
    quad<my_container> q(square{
        {0,0}, 
        128
    }, 32);

    std::vector<quad_entity> entities;

    for (int i = 0; i < 128; i++)
        for (int j = 0; j < 128; j++)
            entities.push_back(quad_entity{i,j});

    for (auto& qe : entities)
        q.insert(&qe);

    for (auto _ : state)
    {
        int i = 0;
        q.query(point{14,14}, 0, [&](const my_container* container) {
            container->for_each([&](const quad_entity* e) {
                if (e->m_x == 14 && e->m_y == 14) i++;
            });
        });

        assert(i == 1);
    }
}

static void BM_MapFind(benchmark::State &state)
{
    std::map<unsigned int, const quad_entity*> map;

    std::vector<quad_entity> entities;

    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            entities.push_back(quad_entity{i,j});
            map.insert(std::make_pair((unsigned int)i*128+j, &entities[i*128+j]));
        }
    }

    for (auto _ : state)
    {
        int i = 0;

        auto result = map.find(63*128+64);
        if (result != map.end())
            i++;

        assert(i == 1);
    }
}

static void BM_MapFindByCoord(benchmark::State &state)
{
    std::map<unsigned int, const quad_entity*> map;

    std::vector<quad_entity> entities;

    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            entities.push_back(quad_entity{i,j});
            map.insert(std::make_pair((unsigned int)i*128+j, &entities[i*128+j]));
        }
    }

    for (auto _ : state)
    {
        int i = 0;

        auto result = std::find_if(map.begin(), map.end(), [&](const std::pair<unsigned int, const quad_entity*>& pair) -> bool {
            return pair.second->m_x == 127 && pair.second->m_y == 127;
        });

        if (result != map.end())
            i++;

        assert(i == 1);
    }
}

// 2x slower to traverse quad
BENCHMARK(BM_QuadTreeTraverse)->RangeMultiplier(2)->Range(128, 128*4);
BENCHMARK(BM_MapTraverse);

// Map find by index is 2000x faster O(n) vs O(logn)
BENCHMARK(BM_QuadFind)->RangeMultiplier(2)->Range(1, 128);
BENCHMARK(BM_MapFind);

// Quad find by coord is 600x faster O(n) vs O(logn)
BENCHMARK(BM_QuadFindByCoord);
BENCHMARK(BM_MapFindByCoord);

// BENCHMARK(BM_QuadTree_QueryWithMaxEntityAmount)
//     ->RangeMultiplier(2)->Range(1, 128*128);

//BENCHMARK(BM_QuadTree_Query)->Arg(16)->Arg(8)->Arg(4)->Arg(2)->Arg(1);
//BENCHMARK(BM_QuadTree_Move)->Arg(16)->Arg(8)->Arg(4)->Arg(2)->Arg(1);