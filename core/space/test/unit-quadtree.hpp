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

TEST(QuadTree, Insert)
{
    quad<my_container> q({
        //{0,0}, {128,128}
        {0,0}, 128
    });

    std::vector<quad_entity> entities {
        {21,32},//12
        {37,35},//28
        {37,34},//27
    };

    q.insert(&entities[0]);
    q.insert(&entities[1]);
    q.insert(&entities[2]);

    // Query all containers in circle range
    int distance=28;
    point center{11,24};

    q.query(center, distance, [&](const my_container* container) {
        container->for_each([&](const quad_entity* e) {
            //if (e->distance(center) <= distance) 
            //    printf("(%d;%d)[%d]\n", e->m_x, e->m_y, e->distance(11,24));
        });
    });

    q.remove(&entities[2]);
    q.remove(&entities[1]);
    q.remove(&entities[0]);
}