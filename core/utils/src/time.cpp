#include <bango/utils/time.h>

using namespace bango::utils;

unsigned long long GetTickCount()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

time::point time::now() noexcept
{
    return time::point(time::duration(GetTickCount()));
}