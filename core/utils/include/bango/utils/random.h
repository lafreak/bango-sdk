#pragma once

namespace bango { namespace utils {

    class random
    {
    public:
        static unsigned int between(unsigned int min, unsigned int max);
        static int between_signed(int min, int max);
    };

}}