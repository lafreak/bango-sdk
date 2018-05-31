# bango-sdk

## Build
### Install dependencies
```
$ sudo apt-get install libmysqlclient-dev
```
### Run CMake
```
$ cd build
$ cmake ..
$ make
$ ./dbserver
$ ./gameserver
```
### Tests & Benchmarks
```
$ cmake .. -DUSE_GTEST=ON -DUSE_BENCHMARK=ON
$ make
$ ./bangonetwork_test
$ ./bangonetwork_benchmark
$ ./bangospace_test
$ ./bangospace_benchmark
```

# Core API
## Table of contents
- [Network](#bangonetwork)
    - [Server](#1-server)
    - [Client](#2-client)
- [Space](#bangospace)
    - [Quadtree](#1-quadtree)
- [Processor](#bangoprocessor)
    - [DB Object](#1-db-object)

## `bango::network`
### 1. Server
Sample usage:

```cpp
#include <bango/network/server.h>
#include <iostream>

using namespace bango::network;

// Class that will be created when new client is connected.
// Must inherit from wrtiable and authorizable.
struct User : public writable, public authorizable {
    using writable::writable;
}

int main()
{
    server<User> serv;

    const unsigned char SOME_EVENT=30; // [0-255]

    // When User sends SOME_EVENT, lambda function will execute.
    serv.when(SOME_EVENT, [&](const std::unique_ptr<User>& user, packet& p) {

        // Get data out of incoming packet in few different ways.
        auto number = p.pop<int>();

        char byte;
        p >> byte;

        std::string message = p.pop_str();

        // Pack outgoing data in few different ways.
        const unsigned char OUTGOING_EVENT=40; // [0-255]
        packet out(OUTGOING_EVENT);

        out << (char) 45 << (int) 1000 << "Hello World!";

        out.push<short>(25);
        out.push_str("This is test message.");

        // Send data
        user->write(out);

        // Or send it all at once
        user->write(OUTGOING_EVENT, "bwdsI", 2, 30000, 400000, "Hey!", 100000000);
        // where b=1byte, w=2byte, d=4byte, s=string, I=8byte

        // Or send empty event
        user->write(OUTGOING_EVENT);
    });

    serv.on_connected([&](const std::unique_ptr<User>& user) {
        std::cout "Connected\n";
        //user->write()...
    });

    serv.on_disconnected([&](const std::unique_ptr<User>& user) {
        std::cout "Disconnected\n";
        //user->write()...
    });

    serv.start("localhost", 3000);

    // Don't close the app.
    std::cin.get();
    return 0;
}
```

### 2. Client
Sample application:

```cpp
#include <bango/network/client.h>
#include <iostream>

using namespace bango::network;

int main()
{
    client my_client;

    // [0-255]
    const unsigned char SOME_EVENT=30; 
    const unsigned char OUTGOING_EVENT=40;

    // When server sends SOME_EVENT, lambda function will execute.
    my_client.when(SOME_EVENT, [&](packet& p) {

        // Print message on console.
        std::cout << p.pop_str() << std::endl;

        my_client.write(OUTGOING_EVENT, "s", "I message back!");
    });

    my_client.connect("localhost", 3000);

    // Don't close the app.
    std::cin.get();
    return 0;
}
```

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

This is the simplest implementation of `quad_entity_container` that uses single list as container.
It is possible to use this container as follows:

```cpp
#include <bango/space/quadtree.h>

using namespace bango::space;

// my_custom_container implementation

void main()
{
    quad<my_custom_container> q(
        // space boundaries
        square {
            {0,0},  // bottom left corner of square space
            128     // square width
        }
    );

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

## `bango::processor`
### 1. DB Object

DB Object represents single record in config file.  
Loading group of objects is as simple as inheriting from `db_object`.  
  
Example config file:
```
(item (index 302) (name "Doggebi Armor") (int 30) (str 20))
(item (index 303) (name "Doggebi Shoes") (hth 10) (wis 5))
...
```

Usage: 
```cpp
#include <bango/processor/db.h>
#include <string>
#include <cstring>
#include <cassert>

using namespace bango::processor;

class Item : public db_object<Item>
{
    unsigned int m_index, m_int, m_str, m_hth, m_wis;
    std::string m_name;

    const std::string& GetName() const { return m_name; }

    // This method must be overriden, index must be unique.
    // It allows to call Item::Find ( index ) to find this item later on.
    unsigned int index() const { return m_index; }

    // This method must be overriden, it gets called for each item property on load.
    virtual void set(lisp::var param) override
    {
        auto attribute = (const char*) param.pop();

        if (std::strcmp(attribute, "index") == 0)
            m_index = param.pop();
        if (std::strcmp(attribute, "name") == 0)
            m_name = (const char*) param.pop();
        if (std::strcmp(attribute, "int") == 0)
            m_int = param.pop();
        if (std::strcmp(attribute, "str") == 0)
            m_str = param.pop();
        if (std::strcmp(attribute, "hth") == 0)
            m_hth = param.pop();
        if (std::strcmp(attribute, "wis") == 0)
            m_wis = param.pop();
    }
};

int main()
{
    const char*         FILE_NAME = "Items.txt";
    const unsigned int  ITEM_INDEX = 303;

    Item::Load(FILE_NAME);

    // DB() is of type std::map, ::at throws exception when index doesnt exist
    auto pItem = Item::DB().at(ITEM_INDEX); 
    
    assert(pItem->GetName() == "Doggebi Shoes");

    return 0;
}
```