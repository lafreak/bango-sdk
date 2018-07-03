#include <bango/utils/random.h>

#include <cstdlib>
#include <ctime>

using namespace bango::utils;

void random::init()
{
    srand(time(0));
}

unsigned int random::between(unsigned int min, unsigned int max)
{
    return (rand() % (max+1 - min)) + min;
}