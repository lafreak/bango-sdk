#include <benchmark/benchmark.h>
#include <bango/space/map.h>
#include <bango/space/quadtree.h>

using namespace bango::space;

#include "quadtree.hpp"

static void BM_MakeCircleInTile(benchmark::State &state)
{
    map m(400, 400, 1024, 1024);
    dynamic_entity e{1023, 1023, ""};

    m.insert(&e);

    for (auto _ : state)
    {
        m.move(&e, 1022, 1023);
        m.move(&e, 1022, 1022);
        m.move(&e, 1023, 1022);
        m.move(&e, 1023, 1023);
    }

    /*  |           |
        |     o o   |
        |     o o   |
        |           |
        |           |
    */
}

static void BM_MakeCircleAroundTiles(benchmark::State &state)
{
    map m(400, 400, 1024, 1024);
    dynamic_entity e{1023, 1023, ""};

    m.insert(&e);

    for (auto _ : state)
    {
        m.move(&e, 1024, 1023);
        m.move(&e, 1024, 1024);
        m.move(&e, 1024, 1023);
        m.move(&e, 1023, 1023);
    }

    /*      |    
           o|o   
       -----|-----
           o|o   
            |    
    */
}

//BENCHMARK(BM_MakeCircleInTile);
//BENCHMARK(BM_MakeCircleAroundTiles);

BENCHMARK_MAIN();