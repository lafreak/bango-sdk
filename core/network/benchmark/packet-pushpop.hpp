static void BM_PacketPushPopInt(benchmark::State &state)
{
    auto p = std::make_shared<bango::network::packet>(10);
    
    for (auto _ : state)
    {
        try
        {
            p->push<int>(20);
            p->pop<int>();
        }
        catch (const std::runtime_error&)
        {
            p = std::make_shared<bango::network::packet>(10);
        }
    }
}

static std::vector<char> PrepareLargeBuffer()
{
    std::vector<char> buffer;

    for (size_t i = 0; i < MAX_PACKET_LENGTH - sizeof(int); i++)
        buffer.push_back(14);

    ((unsigned short*)buffer.data())[0] = MAX_PACKET_LENGTH - sizeof(int);

    return buffer;
}

static void BM_PacketPushPopIntOnLarge(benchmark::State &state)
{
    auto buffer = PrepareLargeBuffer();
    auto p = std::make_shared<bango::network::packet>(buffer);

    for (auto _ : state)
    {
        try
        {
            p->push<short>(20);
            p->pop<short>();
        }
        catch (const std::runtime_error&)
        {
            p = std::make_shared<bango::network::packet>(buffer);
        }
    }
}

static void BM_PacketPushPopStruct(benchmark::State &state)
{
    struct t {
        int a, b; long long c; char d, e, f; int g; char h; int i; int j; long long k=5, l; char m, n, o; short int p, r, s; char t; long long u; int q, w, x; char y, z;
    } my_t;

    auto p = std::make_shared<bango::network::packet>(10);
    
    for (auto _ : state)
    {
        try
        {
            p->push<t>(my_t);
            p->pop<t>();
        }
        catch (const std::runtime_error&)
        {
            p = std::make_shared<bango::network::packet>(10);
        }
    }
}

static void BM_PacketPushPopStructPacked(benchmark::State &state)
{
    struct __attribute__((__packed__)) t {
        int a, b; long long c; char d, e, f; int g; char h; int i; int j; long long k=5, l; char m, n, o; short int p, r, s; char t; long long u; int q, w, x; char y, z;
    } my_t;

    auto p = std::make_shared<bango::network::packet>(10);
    
    for (auto _ : state)
    {
        try
        {
            p->push<t>(my_t);
            p->pop<t>();
        }
        catch (const std::runtime_error&)
        {
            p = std::make_shared<bango::network::packet>(10);
        }
    }
}

static void BM_PacketMerge(benchmark::State &state)
{
    auto p = std::make_shared<bango::network::packet>(std::vector<char>{4, 0, 5, 1});
    auto r = std::make_shared<bango::network::packet>(std::vector<char>{4, 0, 5, 2});

    for (auto _ : state) 
    {
        try
        {
            *p << *r;
            p->pop<char>();
        }
        catch (const std::runtime_error&)
        {
            p = std::make_shared<bango::network::packet>(std::vector<char>{4, 0, 5, 1});
        }
    }
}

static void BM_PacketToVector(benchmark::State &state)
{
    bango::network::packet p({6, 0, 5, 1, 9, 4});

    for (auto _ : state) 
    {
        p.buffer();
    }
}