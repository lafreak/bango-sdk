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

TEST(IntervalMap, AssigningElements)
{
    interval_map<std::uint32_t> im;

    //(group (index 120) (item (100 48 0) (200 1318 0) (350 48 0) (700 1318 0)(1000 1345 0)))
    im.assign(0, 100, 48);
    im.assign(100, 200, 1318);
    im.assign(200, 350, 48);
    im.assign(350, 700, 1318);
    im.assign(700, 1000, 1345);

    EXPECT_EQ(48, im[0]);
    EXPECT_EQ(48, im[50]);
    EXPECT_EQ(48, im[99]);

    EXPECT_EQ(1318, im[100]);
    EXPECT_EQ(1318, im[133]);
    EXPECT_EQ(1318, im[199]);

    EXPECT_EQ(48, im[200]);
    EXPECT_EQ(48, im[277]);
    EXPECT_EQ(48, im[349]);

    EXPECT_EQ(1318, im[350]);
    EXPECT_EQ(1318, im[555]);
    EXPECT_EQ(1318, im[699]);

    EXPECT_EQ(1345, im[700]);
    EXPECT_EQ(1345, im[823]);
    EXPECT_EQ(1345, im[999]);

    EXPECT_EQ(0, im[-1]);
    EXPECT_EQ(0, im[1000]);
    EXPECT_EQ(0, im[5000000]);

}

int main(int argc, char **argv)
{
    random::init();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}