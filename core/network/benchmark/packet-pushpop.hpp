static void BM_PacketPushPopInt(benchmark::State &state)
{
    bango::network::packet p(10);
    
    for (auto _ : state)
    {
        p.push<int>(20);
        p.pop<int>();
    }
}

static void BM_PacketPushPopIntOnLarge(benchmark::State &state)
{
    std::vector<char> buffer;

    for (size_t i = 0; i < MAX_PACKET_LENGTH - sizeof(int); i++)
        buffer.push_back(14);

    ((unsigned short*)buffer.data())[0] = MAX_PACKET_LENGTH - sizeof(int);

    bango::network::packet p(buffer);
    
    for (auto _ : state)
    {
        p.push<int>(20);
        p.pop<int>();
    }
}

static void BM_PacketPushPopStruct(benchmark::State &state)
{
    struct t {
        int a, b; long long c; char d, e, f; int g; char h; int i; int j; long long k=5, l; char m, n, o; short int p, r, s; char t; long long u; int q, w, x; char y, z;
    } my_t;

    bango::network::packet p;
    
    for (auto _ : state)
    {
        p.push<t>(my_t);
        p.pop<t>();
    }
}

static void BM_PacketPushPopStructPacked(benchmark::State &state)
{
    struct __attribute__((__packed__)) t {
        int a, b; long long c; char d, e, f; int g; char h; int i; int j; long long k=5, l; char m, n, o; short int p, r, s; char t; long long u; int q, w, x; char y, z;
    } my_t;

    bango::network::packet p;
    
    for (auto _ : state)
    {
        p.push<t>(my_t);
        p.pop<t>();
    }
}

static void BM_PacketPushPopStructOnLarge(benchmark::State &state)
{
    struct t {
        int a, b; long long c; char d, e, f; int g; char h; int i; int j; long long k=5, l; char m, n, o; short int p, r, s; char t; long long u; int q, w, x; char y, z;
    } my_t;

    std::vector<char> buffer;

    for (size_t i = 0; i < MAX_PACKET_LENGTH - sizeof(t); i++)
        buffer.push_back(14);

    ((unsigned short*)buffer.data())[0] = MAX_PACKET_LENGTH - sizeof(t);

    bango::network::packet p(buffer);
    
    for (auto _ : state)
    {
        p.push<t>(my_t);
        p.pop<t>();
    }
}

static void BM_PacketPushPopStructOnLargePacked(benchmark::State &state)
{
    struct __attribute__((__packed__)) t {
        int a, b; long long c; char d, e, f; int g; char h; int i; int j; long long k=5, l; char m, n, o; short int p, r, s; char t; long long u; int q, w, x; char y, z;
    } my_t;

    std::vector<char> buffer;

    for (size_t i = 0; i < MAX_PACKET_LENGTH - sizeof(t); i++)
        buffer.push_back(14);

    ((unsigned short*)buffer.data())[0] = MAX_PACKET_LENGTH - sizeof(t);

    bango::network::packet p(buffer);
    
    for (auto _ : state)
    {
        p.push<t>(my_t);
        p.pop<t>();
    }
}

static void BM_PacketMerge(benchmark::State &state)
{
    bango::network::packet p({4, 0, 5, 1});
    bango::network::packet r({4, 0, 5, 2});

    for (auto _ : state) 
    {
        p << r;
        p.pop<char>();
    }
}
