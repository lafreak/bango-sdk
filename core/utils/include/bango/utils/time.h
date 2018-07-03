#pragma once

#include <chrono>

namespace bango { namespace utils {

    struct time
    {
        typedef unsigned long long                      rep;
        typedef std::milli                              period;
        typedef std::chrono::duration<rep, period>      duration;
        typedef std::chrono::time_point<time>           point;
        static const bool is_steady =                   true;

        static point now() noexcept;
    };

}}