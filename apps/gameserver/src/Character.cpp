#include "Character.h"

void Character::SetDirection(std::int8_t delta_x, std::int8_t delta_y)
{
    if (delta_x == 0 && delta_y == 0) return;

    float absolute_x = abs(delta_x);
    float absolute_y = abs(delta_y);

    if (absolute_x >= absolute_y && absolute_x > 127) {
        delta_y = 127 * delta_y / absolute_x;
        delta_x = (((delta_x <= 0) - 1) & 0xFE) - 127;
    }
    else if (absolute_x < absolute_y && absolute_y > 127) {
        delta_x = 127 * delta_x / absolute_y;
        delta_y = (((delta_y <= 0) - 1) & 0xFE) - 127;
    }

    m_dir = delta_y + ((delta_x << 8) & 0xFF00);
}