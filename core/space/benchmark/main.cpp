#include <benchmark/benchmark.h>
#include <bango/space.h>

static void BM_MakeCircleInTile(benchmark::State &state)
{
    bango::space::map m(400, 400, 1024, 1024);
    bango::space::dynamic_entity e{1023, 1023, ""};

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
    bango::space::map m(400, 400, 1024, 1024);
    bango::space::dynamic_entity e{1023, 1023, ""};

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

BENCHMARK(BM_MakeCircleInTile);
BENCHMARK(BM_MakeCircleAroundTiles);

BENCHMARK_MAIN();