#include <gtest/gtest.h>

#include <bango/utils/random.h>
#include <bango/utils/time.h>

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

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}