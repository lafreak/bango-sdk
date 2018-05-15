static void BM_QPacketCreationDefault(benchmark::State &state)
{
    for (auto _ : state)
        bango::network::qpacket p;
}

static void BM_QPacketCreationWithType(benchmark::State &state)
{
    for (auto _ : state)
        bango::network::qpacket p(10);
}

static void BM_QPacketCreationWithBuffer(benchmark::State &state)
{
    std::vector<char> buffer;

    for (size_t i = 0; i < MAX_PACKET_LENGTH; i++)
        buffer.push_back(14);

    ((unsigned short*)buffer.data())[0] = MAX_PACKET_LENGTH;

    for (auto _ : state)
        bango::network::qpacket p(buffer);
}

