#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <list>

#include "../include/WorldMap.h"

#include "unit-worldmap.hpp"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}