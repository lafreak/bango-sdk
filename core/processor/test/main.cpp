#include <gtest/gtest.h>
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

struct FilterExample : public db_object<FilterExample>
{
    unsigned short Index;
    std::string Text;

    virtual unsigned int index() const override { return Index; }
    
    virtual void set(lisp::var param) override
    {
        switch (FindAttribute(param.pop()))
        {
            case A_INDEX: Index = (int) param.pop(); break;
            case A_TEXT:
                Text = (const char*) param.pop();
                break;
        }
    }
};

TEST(DBExample, LoadAndFind)
{
    Example::Load("Test/Test.txt");
    auto& e = Example::DB().at(30);//Example::Find(30);

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

TEST(DBExample, LoadAndFindWithFilter)
{
    FilterExample::Load("Test/Test.txt", "filterexample");
    auto& e = FilterExample::DB().at(40);

    EXPECT_EQ(e->Index, 40);
    EXPECT_EQ(e->Text, "hi");

    EXPECT_EQ(FilterExample::DB().find(30), FilterExample::DB().end());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}