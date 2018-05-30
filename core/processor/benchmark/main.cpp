#include <benchmark/benchmark.h>

#include <bango/processor/db.h>

#include <string>
#include <vector>

using namespace bango::processor;

struct Example
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

    unsigned int index() const { return Index; }

    struct Container : public db<Example, Container>
    {
        Container() 
        { 
            m_attributes = {
                {"index",   A_INDEX},
                {"text",    A_TEXT},
                {"extra",   A_EXTRA},
                {"int",     A_INT},
                {"str",     A_STR},
                {"items",   A_ITEMS},
                {"vec",     A_VEC},
            };
        }

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

        virtual void process(Example& e, lisp::var param) override
        {
            switch (attribute(param.pop()))
            {
                case A_INDEX: e.Index = (int) param.pop(); break;
                case A_TEXT:
                    {
                        e.Text.Content = (const char*) param.pop();
                        e.Text.Length = (int) param.pop();
                    }
                    break;

                case A_EXTRA:
                    {
                        while (param.consp())
                        {
                            lisp::var p = param.pop();
                            switch (attribute(p.pop()))
                            {
                                case A_INT: e.Int = (int) p.pop(); break;
                                case A_STR: e.Str = (int) p.pop(); break;
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

                            e.Items.push_back(Item{id,amount,bound});
                        }
                    }
                    break;

                case A_VEC:
                    {
                        while (param.consp())
                            e.Numbers.push_back( (int) param.pop());
                    }
                    break;
            }
        }

    };

    static Example*     Find(unsigned int index)    { return Container::find(index); }
    static bool         Load(const char* name)      { return Container::load(name); }
};

static void BM_DBLoad(benchmark::State &state)
{
    for (auto _ : state)
        Example::Load("Test.txt");
}

static void BM_DBFind(benchmark::State &state)
{
    Example::Load("Test.txt");

    Example *k;

    for (auto _ : state)
        benchmark::DoNotOptimize(k = Example::Find(30));
}

BENCHMARK(BM_DBLoad);
BENCHMARK(BM_DBFind);

BENCHMARK_MAIN();