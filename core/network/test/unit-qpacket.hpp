TEST(QPacketTest, DefaultConstructor)
{
    qpacket p;
    EXPECT_EQ(3, p.size());
    EXPECT_EQ(0, p.type());
}

TEST(QPacketTest, TypeConstructor)
{
    qpacket p(24);
    EXPECT_EQ(3, p.size());
    EXPECT_EQ(24, p.type());
}

TEST(QPacketTest, RawConstructor)
{
    qpacket p({7, 0, 2, 5, 1, 0, 8});
    EXPECT_EQ(7, p.size());
    EXPECT_EQ(2, p.type());
}

TEST(QPacketTest, ChangeType)
{
    qpacket p(2);
    EXPECT_EQ(2, p.type());
    p.change_type(3);
    EXPECT_EQ(3, p.type());
    p.change_type(4).change_type(5).change_type(6);
    EXPECT_EQ(6, p.type());
}

TEST(QPacketTest, GenericPushPop)
{
    qpacket p;
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

TEST(QPacketTest, StringPushPop)
{
    qpacket p;
    p.push_str("Hello World!");
    p.push_str("");
    p.push_str("Second Message");

    EXPECT_EQ("Hello World!", p.pop_str());
    EXPECT_EQ("", p.pop_str());
    EXPECT_EQ("Second Message", p.pop_str());
}

TEST(QPacketTest, StreamOperators)
{
    qpacket p;
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

TEST(QPacketTest, Merge)
{
    qpacket p(3);
    p.push_str("First Packet");
    qpacket r(5);
    r.push_str("Second Packet");
    qpacket s(10);
    s.push_str("Third Packet");

    p.merge(r);
    p << s;

    EXPECT_EQ(3, p.type());
    EXPECT_EQ("First Packet", p.pop_str());
    EXPECT_EQ("Second Packet", p.pop_str());
    EXPECT_EQ("Third Packet", p.pop_str());
}


TEST(QPacketTest, InOutExceptions)
{
    qpacket p;

    for (unsigned short i = 3; i < MAX_PACKET_LENGTH; i++)
        EXPECT_NO_THROW(p.push<char>(7));
    
    EXPECT_THROW(p.push<char>(13), std::runtime_error);

    for (unsigned short i = MAX_PACKET_LENGTH-1; i >= 3; i--)
        EXPECT_NO_THROW(p.pop<char>());

    EXPECT_THROW(p.pop<char>(), std::runtime_error);
}


TEST(QPacketTest, ToVector)
{
    qpacket p(4);

    p.push<char>(1);
    p.push<char>(2);
    p.push<char>(3);
    p.push<char>(4);

    auto buffer = p.buffer();
    EXPECT_EQ(7, buffer[0]);
    EXPECT_EQ(0, buffer[1]);
    EXPECT_EQ(4, buffer[2]);
    EXPECT_EQ(1, buffer[3]);
    EXPECT_EQ(2, buffer[4]);
    EXPECT_EQ(3, buffer[5]);
    EXPECT_EQ(4, buffer[6]);
}