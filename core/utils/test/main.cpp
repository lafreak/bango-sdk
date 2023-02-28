#include <gtest/gtest.h>

#include <bango/utils/random.h>
#include <bango/utils/time.h>
#include <bango/utils/interval_map.h>

#include <thread>

using namespace bango::utils;

TEST(RandomTest, Between)
{
    constexpr unsigned int MIN = 315;
    constexpr unsigned int MAX = 324;
    constexpr unsigned int TRY = 300000;

    for (int i = 0; i < TRY; i++)
    {
        auto r = random::between(MIN, MAX);
        EXPECT_EQ(true, r >= MIN && r <= MAX);
    }
}

TEST(ClockTest, DISABLED_Difference)
{
    auto t0 = time::now();
    std::this_thread::sleep_until(t0 + std::chrono::seconds(1));
    auto t1 = time::now();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto t2 = time::now();

    EXPECT_TRUE(((t1-t0).count() >= 1000) && ((t1-t0).count() <= 1010));
    EXPECT_TRUE(((t2-t1).count() >= 2000) && ((t2-t1).count() <= 2010));
    EXPECT_TRUE(((t2-t0).count() >= 3000) && ((t2-t0).count() <= 3010));
}

TEST(interval_map, EmptyMapTest)
{
    interval_map<std::uint32_t> m;
    for(int i = 0; i < 100; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, TryToAddTheSameFirstMapElementAs_m_valBegin)
{
    interval_map<std::uint32_t> m;
    m.assign(5, 10, 0);

    for(int i = 0; i < 100; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, TryToAddTheSameLastElementAs_m_valBegin)
{
    interval_map<std::uint32_t> m;
    m.assign(5, 10, 20);
    m.assign(12, 20, 0);

    for(int i = 0; i < 5; i++)
        EXPECT_EQ(m[i], 0);

    for(int i = 5; i < 10; i++)
        EXPECT_EQ(m[i], 20);

    for(int i = 10; i < 50; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, TryToAddTheSameFirstMapElementAs_m_valBeginWhenOneElementIsAlreadyInMap)
{
    interval_map<std::uint32_t> m;
    m.assign(10, 20, 20);
    m.assign(3, 7, 0);

    for(int i = 0; i < 10; i++)
        EXPECT_EQ(m[i], 0);

    for(int i = 10; i < 20; i++)
        EXPECT_EQ(m[i], 20);

    for(int i = 20; i < 100; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, OverwriteRangeWithDifferentValue)
{
    interval_map<std::uint32_t> m;
    m.assign(10, 20, 20);
    m.assign(10, 20, 30);

    for(int i = 0; i < 10; i++)
        EXPECT_EQ(m[i], 0);

    for(int i = 10; i < 20; i++)
        EXPECT_EQ(m[i], 30);

    for(int i = 20; i < 100; i++)
        EXPECT_EQ(m[i], 0);
}


TEST(interval_map, AddSingleRangeToEmptyMap)
{
    interval_map<std::uint32_t> m;
    m.assign(20, 50, 20);

    for (int i = 0; i < 20; i++)
        EXPECT_EQ(m[i], 0);

    for (int i = 20; i < 50; i++)
        EXPECT_EQ(m[i], 20);

    for (int i = 50; i < 60; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, Add2RangesNotOverlappingEachother)
{
    interval_map<std::uint32_t> m;
    m.assign(20, 50, 20);
    m.assign(60, 80, 30);

    for (int i = 0; i < 20; i++)
        EXPECT_EQ(m[i], 0);

    for (int i = 20; i < 50; i++)
        EXPECT_EQ(m[i], 20);

    for (int i = 50; i < 60; i++)
        EXPECT_EQ(m[i], 0);

    for (int i = 60; i < 80; i++)
        EXPECT_EQ(m[i], 30);

    for (int i = 80; i < 100; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, Add2RangesOverlappingEachotherSmallerOverBigger)
{
    interval_map<std::uint32_t> m;
    m.assign(50, 100, 20);
    m.assign(60, 80, 30);

    for (int i = 0; i < 50; i++)
        EXPECT_EQ(m[i], 0);

    for (int i = 50; i < 60; i++)
        EXPECT_EQ(m[i], 20);

    for (int i = 60; i < 80; i++)
        EXPECT_EQ(m[i], 30);

    for (int i = 80; i < 100; i++)
        EXPECT_EQ(m[i], 20);

    for (int i = 100; i < 130; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, Add2RangesOverlappingEachotherBiggerOverSmaller)
{
    interval_map<std::uint32_t> m;
    m.assign(60, 80, 20);
    m.assign(50, 100, 30);

    for (int i = 0; i < 50; i++)
        EXPECT_EQ(m[i], 0);

    for (int i = 50; i < 100; i++)
        EXPECT_EQ(m[i], 30);

    for (int i = 100; i < 110; i++)
        EXPECT_EQ(m[i], 0);
}


TEST(interval_map, TwoRangesPartialOverlapOnBeggining)
{
    interval_map<std::uint32_t> m;
    m.assign(50, 100, 20);
    m.assign(40, 70, 30);

    for (int i = 0; i < 40; i++)
        EXPECT_EQ(m[i], 0);

    for (int i = 40; i < 70; i++)
        EXPECT_EQ(m[i], 30);

    for (int i = 70; i < 100; i++)
        EXPECT_EQ(m[i], 20);

    for (int i = 100; i < 110; i++)
        EXPECT_EQ(m[i], 0);
}


TEST(interval_map, TwoRangesPartialOverlapOnTheEnd)
{
    interval_map<std::uint32_t> m;
    m.assign(50, 100, 20);
    m.assign(90, 110, 30);

    for (int i = 0; i < 50; i++)
        EXPECT_EQ(m[i], 0);

    for (int i = 50; i < 90; i++)
        EXPECT_EQ(m[i], 20);

    for (int i = 90; i < 110; i++)
        EXPECT_EQ(m[i], 30);

    for (int i = 110; i < 120; i++)
        EXPECT_EQ(m[i], 0);
}


TEST(interval_map, TwoRangesPartialOverlapOnTheEndSameValue)
{
    interval_map<std::uint32_t> m;
    m.assign(50, 100, 20);
    m.assign(90, 110, 20);

    for (int i = 0; i < 50; i++)
        EXPECT_EQ(m[i], 0);

    for (int i = 50; i < 110; i++)
        EXPECT_EQ(m[i], 20);

    for (int i = 110; i < 120; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, TwoRangesPartialOverlapOnBegginingSameValue)
{
    interval_map<std::uint32_t> m;
    m.assign(50, 100, 20);
    m.assign(40, 70, 20);

    for (int i = 0; i < 40; i++)
        EXPECT_EQ(m[i], 0);

    for (int i = 40; i < 100; i++)
        EXPECT_EQ(m[i], 20);

    for (int i = 100; i < 110; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, AssignTwoRangesOverlappingEachOtherWithSameValue)
{
    interval_map<std::uint32_t> m;
    m.assign(50, 100, 20);
    m.assign(40, 70, 20);

    for (int i = 0; i < 40; i++)
        EXPECT_EQ(m[i], 0);

    for (int i = 40; i < 100; i++)
        EXPECT_EQ(m[i], 20);

    for (int i = 100; i < 110; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, AssignRangeInsideOtherRangeWithSameValue)
{
    interval_map<std::uint32_t> m;
    m.assign(50, 100, 20);
    m.assign(60, 90, 20);

    for (int i = 0; i < 50; i++)
        EXPECT_EQ(m[i], 0);

    for (int i = 50; i < 100; i++)
        EXPECT_EQ(m[i], 20);

    for (int i = 100; i < 110; i++)
        EXPECT_EQ(m[i], 0);
}


TEST(interval_map, DecreaseInterval)
{
    interval_map<std::uint32_t> m;
    m.assign(50, 100, 20);
    m.assign(40, 70, 0);

    for (int i = 0; i < 70; i++)
        EXPECT_EQ(m[i], 0);

    for (int i = 70; i < 100; i++)
        EXPECT_EQ(m[i], 20);

    for (int i = 100; i < 200; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, ClearInterval)
{
    interval_map<std::uint32_t> m;
    m.assign(50, 100, 20);
    m.assign(0, 200, 0);

    for (int i = 0; i < 200; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, KeyBeginGreaterThanKeyEnd)
{
    interval_map<std::uint32_t> m;
    m.assign(100, 50, 20);

    for (int i = 0; i < 200; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, KeyBeginEqualKeyEnd)
{
    interval_map<std::uint32_t> m;
    m.assign(100, 100, 20);

    for (int i = 0; i < 200; i++)
        EXPECT_EQ(m[i], 0);
}


TEST(interval_map, KeyBeginIsEqualToKeyEndAndThenAssignNormalRanges)
{
    interval_map<std::uint32_t> m;
    m.assign(100, 100, 20);
    m.assign(50, 70, 30);
    m.assign(55, 65, 40);

    for(int i = 0; i< 50; i++)
        EXPECT_EQ(m[i], 0);

    for(int i = 50; i< 55; i++)
        EXPECT_EQ(m[i], 30);

    for(int i = 55; i< 65; i++)
        EXPECT_EQ(m[i], 40);

    for(int i = 65; i< 70; i++)
        EXPECT_EQ(m[i], 30);

    for(int i = 70; i< 200; i++)
        EXPECT_EQ(m[i], 0);
}


TEST(interval_map, PartlyOverwriteRangeWithTwoOtherRanges)
{
    interval_map<std::uint32_t> m;
    m.assign(20, 40, 20);
    m.assign(35, 60, 30);
    m.assign(30, 50, 40);

    for(int i = 0; i< 20; i++)
        EXPECT_EQ(m[i], 0);

    for(int i = 20; i< 30; i++)
        EXPECT_EQ(m[i], 20);

    for(int i = 30; i< 50; i++)
        EXPECT_EQ(m[i], 40);

    for(int i = 50; i< 60; i++)
        EXPECT_EQ(m[i], 30);

    for(int i = 60; i< 200; i++)
        EXPECT_EQ(m[i], 0);
}


TEST(interval_map, IncreaseRangeWithSameValue)
{
    interval_map<std::uint32_t> m;
    m.assign(20, 40, 20);
    m.assign(40, 60, 20);

    for(int i = 0; i< 20; i++)
        EXPECT_EQ(m[i], 0);

    for(int i = 20; i< 60; i++)
        EXPECT_EQ(m[i], 20);

    for(int i = 60; i< 200; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, IncreaseRangeWithSameValue2)
{
    interval_map<std::uint32_t> m;
    m.assign(40, 60, 20);
    m.assign(20, 40, 20);
    for(int i = 0; i< 20; i++)
        EXPECT_EQ(m[i], 0);

    for(int i = 20; i< 60; i++)
        EXPECT_EQ(m[i], 20);

    for(int i = 60; i< 200; i++)
        EXPECT_EQ(m[i], 0);
}

TEST(interval_map, AssignOneRangeAndAddOtherRangeWithDifferentValueUntilKeyBeginOfFirstOne)
{
    interval_map<std::uint32_t> m;
    m.assign(40, 60, 20);
    m.assign(20, 40, 30);

    for(int i = 0; i< 20; i++)
        EXPECT_EQ(m[i], 0);

    for(int i = 20; i< 40; i++)
        EXPECT_EQ(m[i], 30);

    for(int i = 40; i< 60; i++)
        EXPECT_EQ(m[i], 20);

    for(int i = 60; i< 200; i++)
        EXPECT_EQ(m[i], 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}