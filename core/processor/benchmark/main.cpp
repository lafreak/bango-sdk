#include <benchmark/benchmark.h>

#include <bango/processor/db.h>

#include <string>
#include <vector>

using namespace bango::processor;

enum ATTRIBUTES
{
    A_NONE,
    A_INDEX,
    A_TEXT,
    A_EXTRA,
    A_INT,
    A_STR,
    A_ITEMS,
    A_VEC,
};

static std::map<std::string, unsigned int> g_attributes = 
{
    {"index",   A_INDEX},
    {"text",    A_TEXT},
    {"extra",   A_EXTRA},
    {"int",     A_INT},
    {"str",     A_STR},
    {"items",   A_ITEMS},
    {"vec",     A_VEC},
};

static unsigned int FindAttribute(const char* attribute)
{
    auto result = g_attributes.find(attribute);
    if (result != g_attributes.end())
        return result->second;
    return 0;
}

struct Example : public db_object<Example>
{
    unsigned short  Index;

    struct {
        std::string     Content;
        char            Length;
    } Text;

    char Int;
    char Str;

    struct Item
    {
        unsigned short  Index;
        unsigned int    Amount;
        bool            Bound;
    };

    std::vector<Item> Items;
    std::vector<char> Numbers;

    virtual unsigned int index() const override { return Index; }
    
    virtual void set(lisp::var param) override
    {
        switch (FindAttribute(param.pop()))
        {
            case A_INDEX: Index = (int) param.pop(); break;
            case A_TEXT:
                {
                    Text.Content = (const char*) param.pop();
                    Text.Length = (int) param.pop();
                }
                break;

            case A_EXTRA:
                {
                    while (param.consp())
                    {
                        lisp::var p = param.pop();
                        switch (FindAttribute(p.pop()))
                        {
                            case A_INT: Int = (int) p.pop(); break;
                            case A_STR: Str = (int) p.pop(); break;
                        }
                    }
                }
                break;

            case A_ITEMS:
                {
                    while (param.consp())
                    {
                        lisp::var p = param.pop();
                        unsigned short id =       (int) p.pop();
                        unsigned int amount =   (int) p.pop();
                        bool bound =    ((int) p.pop()) == 1;

                        Items.push_back(Item{id,amount,bound});
                    }
                }
                break;

            case A_VEC:
                {
                    while (param.consp())
                        Numbers.push_back( (int) param.pop());
                }
                break;
        }
    }
};

static void BM_DBLoad(benchmark::State &state)
{
    for (auto _ : state)
        Example::Load("Test/Test.txt");
}

static void BM_DBFind(benchmark::State &state)
{
    Example::Load("Test/Test.txt");

    const Example *k;

    for (auto _ : state)
        benchmark::DoNotOptimize(k = Example::Find(30));
}

BENCHMARK(BM_DBLoad);
BENCHMARK(BM_DBFind);

BENCHMARK_MAIN();