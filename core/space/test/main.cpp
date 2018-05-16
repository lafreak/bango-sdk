#include <gtest/gtest.h>
//#include <bango/space.h>
#include <bango/space/quadtree.h> // includes map for now

#include <exception>
#include <list>
#include <memory>

using namespace bango::space;

#include "unit-map.hpp"
#include "unit-quadtree.hpp"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}