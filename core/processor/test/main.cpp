#include <gtest/gtest.h>
#include <bango/processor/db.h>

#include <iostream>
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

TEST(DBExample, LoadAndFind)
{
    Example::Load("Test.txt");
    Example* e = Example::Find(30);

    EXPECT_EQ(30, e->Index);
    EXPECT_EQ("HELLO WORLD", e->Text.Content);
    EXPECT_EQ(35, e->Text.Length);
    EXPECT_EQ(2, e->Int);
    EXPECT_EQ(3, e->Str);

    ASSERT_EQ(2, e->Items.size());

    EXPECT_EQ(100, e->Items[0].Index);
    EXPECT_EQ(2, e->Items[0].Amount);
    EXPECT_EQ(false, e->Items[0].Bound);

    EXPECT_EQ(101, e->Items[1].Index);
    EXPECT_EQ(3, e->Items[1].Amount);
    EXPECT_EQ(true, e->Items[1].Bound);

    ASSERT_EQ(6, e->Numbers.size());

    EXPECT_EQ(3, e->Numbers[0]);
    EXPECT_EQ(3, e->Numbers[1]);
    EXPECT_EQ(4, e->Numbers[2]);
    EXPECT_EQ(8, e->Numbers[3]);
    EXPECT_EQ(8, e->Numbers[4]);
    EXPECT_EQ(10, e->Numbers[5]);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}