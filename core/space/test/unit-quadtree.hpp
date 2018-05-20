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

namespace {
    class QuadTreeQuery : public ::testing::Test
    {
    protected:
        quad<my_container> *q;
        std::vector<quad_entity> entities;

        std::list<int> results;
        std::vector<std::pair<int,int>> expected;

        point center{11,24};

        virtual void SetUp()
        {
            q = new quad<my_container>(square{
                {0,0}, 
                128
            });

            entities = std::vector<quad_entity> {
                {21 ,32 },//12
                {1  ,32 },//12
                {37 ,34 },//27
                {37 ,35 },//28
                {41 ,0  },//38
                {23 ,99 },//75
                {99 ,23 },//88
                {102,9  },//92
                {128,0  },//119
                {100,124},//133
                {128,128},//156
            };

            expected = std::vector<std::pair<int,int>>{
                // (distance, size)
                std::make_pair(0, 0),
                std::make_pair(11, 0),
                std::make_pair(12, 2),
                std::make_pair(26, 2),
                std::make_pair(27, 3),
                std::make_pair(28, 4),
                std::make_pair(37, 4),
                std::make_pair(38, 5),
                std::make_pair(74, 5),
                std::make_pair(75, 6),
                std::make_pair(87, 6),
                std::make_pair(88, 7),
                std::make_pair(91, 7),
                std::make_pair(92, 8),
                std::make_pair(118, 8),
                std::make_pair(119, 9),
                std::make_pair(132, 9),
                std::make_pair(133, 10),
                std::make_pair(155, 10),
                std::make_pair(156, 11),
                std::make_pair(9999, 11),
            };

            for (auto& e : entities)
                q->insert(&e);
        }

        virtual void TearDown()
        {
            delete q;
        }

        void Query(point center, int distance) 
        {
            q->query(center, distance, [&](const my_container* container) {
                container->for_each([&](const quad_entity* e) {
                    if (e->distance(center) <= distance)
                        results.push_back(e->distance(center));
                });
            });
        }

    };

    TEST_F(QuadTreeQuery, Query)
    {
        for (auto &pair : expected)
        {
            Query(center, pair.first);
            ASSERT_THAT(results, ::testing::SizeIs(pair.second));
            results.clear();
        }
    }
}

TEST(QuadTreeQuery_, DuplicatesSafe)
{
    quad<my_container> q(square{
        {0,0}, 
        128
    });

    std::vector<quad_entity> duplicates;
    for (int i = 0; i < QUADTREE_MAX_NODES+1; i++)
        duplicates.push_back(quad_entity{0,0}); 

    for (int i = 0; i < QUADTREE_MAX_NODES; i++)
        EXPECT_NO_THROW(q.insert(&duplicates[i]));
    
#ifdef DUPLICATES_SAFE
    EXPECT_NO_THROW(q.insert(&duplicates[QUADTREE_MAX_NODES]));
    EXPECT_EQ(QUADTREE_MAX_NODES+1, q.size());
    EXPECT_EQ(1, q.distinct_size());
#else
    EXPECT_EQ(QUADTREE_MAX_NODES, q.size());
    EXPECT_THROW(q.insert(&duplicates[QUADTREE_MAX_NODES]), std::runtime_error);
#endif
}