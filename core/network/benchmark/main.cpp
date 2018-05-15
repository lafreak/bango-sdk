#include <benchmark/benchmark.h>
#include <bango/network/packet.h>
#include <bango/network/qpacket.h>

#include <memory>

#include "packet-creation.hpp"
#include "packet-pushpop.hpp"

#include "qpacket-creation.hpp"
#include "qpacket-pushpop.hpp"

BENCHMARK(BM_PacketCreationDefault);
BENCHMARK(BM_QPacketCreationDefault);

BENCHMARK(BM_PacketCreationWithType);
BENCHMARK(BM_QPacketCreationWithType);

BENCHMARK(BM_PacketCreationWithBuffer);
BENCHMARK(BM_QPacketCreationWithBuffer);


BENCHMARK(BM_PacketPushPopInt);
BENCHMARK(BM_QPacketPushPopInt);

BENCHMARK(BM_PacketPushPopIntOnLarge);
BENCHMARK(BM_QPacketPushPopIntOnLarge);

BENCHMARK(BM_PacketPushPopStruct);
BENCHMARK(BM_QPacketPushPopStruct);

BENCHMARK(BM_PacketPushPopStructPacked);
BENCHMARK(BM_QPacketPushPopStructPacked);

BENCHMARK(BM_PacketMerge);
BENCHMARK(BM_QPacketMerge);


BENCHMARK(BM_PacketPushPopStructOnLarge);
BENCHMARK(BM_PacketPushPopStructOnLargePacked);


BENCHMARK(BM_PacketToVector);
BENCHMARK(BM_QPacketToVector);

BENCHMARK_MAIN();