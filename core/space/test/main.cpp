#include <gtest/gtest.h>
#include <bango/space.h>

#include <exception>
#include <list>

using namespace bango::space;

#include "unit-map.hpp"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}