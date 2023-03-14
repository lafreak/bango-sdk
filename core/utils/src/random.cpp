#include <bango/utils/random.h>

#include <random>

using namespace bango::utils;

unsigned int random::between(unsigned int min, unsigned int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> distr(min, max);
    return distr(gen);
}