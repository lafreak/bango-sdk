#include <gtest/gtest.h>

#include <bango/network/packet.h>

using namespace bango::network;

struct t {
    unsigned char a;
    unsigned short b;
    unsigned int c;
    unsigned long long d;
};

#include "unit-packet.hpp"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}