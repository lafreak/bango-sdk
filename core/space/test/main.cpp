#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <bango/space/quadtree.h>

#include <exception>
#include <list>
#include <memory>
#include <algorithm>

using namespace bango::space;

//#include "unit-map.hpp"
#include "unit-quadtree.hpp"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}