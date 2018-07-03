#pragma once

namespace bango { namespace utils {

    class random
    {
    public:
        static void init();

        static unsigned int between(unsigned int min, unsigned int max);
    };

}}