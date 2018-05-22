#include <benchmark/benchmark.h>
#include <bango/network/packet.h>

#include <memory>

#include "packet-creation.hpp"
#include "packet-pushpop.hpp"

BENCHMARK(BM_PacketCreationDefault);
BENCHMARK(BM_PacketCreationWithType);
BENCHMARK(BM_PacketCreationWithBuffer);
BENCHMARK(BM_PacketCreationWithSmallBuffer);
BENCHMARK(BM_PacketPushPopInt);
BENCHMARK(BM_PacketPushPopIntOnLarge);
BENCHMARK(BM_PacketPushPopStruct);
BENCHMARK(BM_PacketPushPopStructPacked);
BENCHMARK(BM_PacketMerge);
BENCHMARK(BM_PacketToVector);

BENCHMARK_MAIN();