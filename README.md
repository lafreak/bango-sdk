# bango-sdk

## Build
### Install dependencies
```
$ sudo apt-get install libmysqlcppconn-dev # not sure if neccessary
$ sudo apt-get install libmysqlclient-dev
```
### Run CMake
```
$ cd build
$ cmake ..
$ make
$ ../bin/dbserver
$ ../bin/gameserver
```
### Tests & Benchmarks
```
$ cmake .. -DUSE_GTEST=ON -DUSE_BENCHMARK=ON
$ make
$ ../bin/bangonetwork_test
$ ../bin/bangonetwork_benchmark
$ ../bin/bangospace_test
$ ../bin/bangospace_benchmark
```

# Core API
## `bango::space`
### 1. Quadtree
Implementation of data structure used to partition a two-dimensional space by recursively subdividing it into four quadrants or regions each of them called `quad`. Each `quad` leaf contains data structure associated with it correspondaing to a specific subregion called `quad_entity_container`. This is base template class and needs to be implemented in order to make use of it.  
  
Example:
```cpp
class my_custom_container : public quad_entity_container<my_custom_container>
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
```

This is the simplest implementation that uses single list as container.
It is possible to use this container as follows:

```cpp
#include <bango/space/quadtree.h>

using namespace bango::space;

// my_custom_container implementation

void main()
{
    quad<my_custom_container> q({
        // space boundaries
        {0,0},      // bottom left corner of 2D space
        {128,128}   // top right corner of 2D space
    });

    // declare entities with its coordinates
    quad_entity e1{25,41};
    quad_entity e2{51,120};

    // insert entities into tree
    q.insert(&e1);
    q.insert(&e2);    

    point center{24,40};
    int radius=2;

    q.query(center, radius, [&](const my_custom_container* container) {
        // This piece of code will execute for containers in range of (center, radius) (AOE)
        container->for_each([&](const quad_entity* e) {
            // This piece of code will execute for all entities in containers of range (center, radius) (AOE)

            if (e->distance(center) <= radius)
            {
                // This piece of code will execute for entities in range
                // In this example for e1
            }

        });
    });

    // remove entities from tree
    q.remove(&e1);
    q.remove(&e2);
}
```