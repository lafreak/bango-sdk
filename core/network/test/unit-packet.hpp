#include <bango/network/packet.h>

using namespace bango::network;

struct t {
    unsigned char a;
    unsigned short b;
    unsigned int c;
    unsigned long long d;
};

TEST(PacketTest, DefaultConstructor)
{
    packet p;
    EXPECT_EQ(3, p.size());
    EXPECT_EQ(0, p.type());
}

TEST(PacketTest, TypeConstructor)
{
    packet p(24);
    EXPECT_EQ(3, p.size());
    EXPECT_EQ(24, p.type());
}

TEST(PacketTest, RawConstructor)
{
    packet p({7, 0, 2, 5, 1, 0, 8});
    EXPECT_EQ(7, p.size());
    EXPECT_EQ(2, p.type());
}

TEST(PacketTest, ChangeType)
{
    packet p(2);
    EXPECT_EQ(2, p.type());
    p.change_type(3);
    EXPECT_EQ(3, p.type());
    p.change_type(4).change_type(5).change_type(6);
    EXPECT_EQ(6, p.type());
}

TEST(PacketTest, GenericPushPop)
{
    packet p;
    p.push<char>(124);
    p.push<short>(30200);
    p.push<int>(2100200400);
    p.push<long long>(4820200400);
    p.push<t>({1, 2, 3, 4});

    EXPECT_EQ(124, p.pop<char>());
    EXPECT_EQ(30200, p.pop<short>());
    EXPECT_EQ(2100200400, p.pop<int>());
    EXPECT_EQ(4820200400, p.pop<long long>());

    auto value = p.pop<t>();
    EXPECT_EQ(1, value.a);
    EXPECT_EQ(2, value.b);
    EXPECT_EQ(3, value.c);
    EXPECT_EQ(4, value.d);
}

TEST(PacketTest, StringPushPop)
{
    packet p;
    p.push_str("Hello World!");
    p.push_str("");
    p.push_str("Second Message");

    EXPECT_EQ("Hello World!", p.pop_str());
    EXPECT_EQ("", p.pop_str());
    EXPECT_EQ("Second Message", p.pop_str());
}

TEST(PacketTest, StreamOperators)
{
    packet p;
    p.push<short>(3);
    p.push<int>(5);
    p.push<char>('@');
    p.push<char>('!');
    p << "Test!";
    p.push<t>({1, 2, 3, 4});

    short a=0;
    int b=0;
    char c=0, d=0;
    std::string e="";
    t value={};

    p >> a >> b >> c >> d >> e >> value;

    EXPECT_EQ(3, a);
    EXPECT_EQ(5, b);
    EXPECT_EQ('@', c);
    EXPECT_EQ('!', d);
    EXPECT_EQ("Test!", e);
    EXPECT_EQ(1, value.a);
    EXPECT_EQ(2, value.b);
    EXPECT_EQ(3, value.c);
    EXPECT_EQ(4, value.d);
}

TEST(PacketTest, Merge)
{
    packet p(3);
    p.push_str("First Packet");
    packet r(5);
    r.push_str("Second Packet");
    packet s(10);
    s.push_str("Third Packet");

    p.merge(r);
    p << s;

    EXPECT_EQ(3, p.type());
    EXPECT_EQ("First Packet", p.pop_str());
    EXPECT_EQ("Second Packet", p.pop_str());
    EXPECT_EQ("Third Packet", p.pop_str());
}

TEST(PacketTest, Valid)
{
    packet p({4, 0, 1});
    EXPECT_FALSE(p.valid());

    packet r({3, 0, 1});
    EXPECT_TRUE(r.valid());
}