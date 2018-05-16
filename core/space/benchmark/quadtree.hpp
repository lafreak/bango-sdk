class my_container : public quad_entity_container<my_container>
{
    std::list<const quad_entity*> m_entities;
public:
    const std::list<const quad_entity*>& entities() const { return m_entities; }

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
    quad<my_container> q({
        {0,0}, {128,128}
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

    // Query all containers in circle range
    int distance=5;
    point center{11,24};

    const quad_entity* k=nullptr;

    for (auto _ : state)
    {
        q.query(center, distance, [&](const my_container* container) {
            container->for_each([&](const quad_entity* e) {
                if (e->distance(center) < distance)
                    k=e;
            });
        });
    }
}

static void BM_QuadTree_QueryAwayFromLure(benchmark::State &state)
{
    quad<my_container> q({
        {0,0}, {128,128}
    });

    std::vector<quad_entity> entities;

    for (int i = 0; i < 64; i+=1)
    {
        for (int j = 0; j < 64; j+=1)
        {
            entities.push_back(quad_entity{i,j});
        }
    }

    for (auto& qe : entities)
        q.insert(&qe);

    int distance=1;
    point center{127,127};

    const quad_entity* k=nullptr;

    for (auto _ : state)
    {
        q.query(center, distance, [&](const my_container* container) {
            for (auto& e : container->entities()) {
                if (e->distance(center) < distance)
                    k=e;
            }
        });
    }
}

BENCHMARK(BM_QuadTree_Query)->Arg(64)->Arg(32)->Arg(16)->Arg(8)->Arg(4)->Arg(2);
BENCHMARK(BM_QuadTree_QueryAwayFromLure);