#include "Character.h"

using namespace bango::utils;

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

std::uint16_t Character::GetResist(std::uint8_t type) const
{
	switch (type)
	{
        case RT_FIRE:
            return GetInteligence() / 9;
        case RT_ICE:
            return GetInteligence() / 9;
        case RT_LITNING:
            return GetInteligence() / 9;
        case RT_PALSY:
            return GetHealth()      / 9;
        case RT_CURSE:
            return GetWisdom()      / 9;
    }
}

bool Character::CheckHit(Character* target, int bonus)
{
    int level_diff = (int) GetLevel() - target->GetLevel();

    if (level_diff > 100)
        level_diff = 100;
    if (level_diff < -100)
        level_diff = -100;

    int otp =0;

    if (level_diff < 0)
        otp -= g_nAddOTPLv[abs(level_diff)];
    else
        otp += g_nAddOTPLv[abs(level_diff)];

    otp += GetHit();
    otp -= target->GetDodge();
    otp += bonus;

    if (otp > 41)
        otp = 41;
    if (otp < -41)
        otp = -41;

    int chance =0;

    if (otp < 0)
        chance = 100 - g_nHitChance[abs(otp)];
    else
        chance = g_nHitChance[otp];

    return random::between(1, 100) <= chance;
}

std::int64_t Character::GetFinalDamage(Character* attacker, std::int64_t damage, bool magical)
{
    int level_diff = (int) attacker->GetLevel() - (int) GetLevel();

    if (level_diff > 100)
        level_diff = 100;
    if (level_diff < -100)
        level_diff = -100;

    if (magical)
    {
        damage += level_diff * abs(level_diff) / 4;
    }
    else
    {
        // if (GetType() == CK_MONSTER || attacker->GetType() == CK_MONSTER)
        if (true/*unknown cond*/ && abs(level_diff) < 100)//TODO: Find condition
        {
            if (level_diff < 0)
                damage -= g_nAddDefLv[abs(level_diff)];
            else
                damage += g_nAddDefLv[abs(level_diff)];
        }

        damage -= GetDefense(attacker->GetAttackType());
        damage *= 1.f - (GetAbsorb() / 100.f);//BUG What if absorb > 100?
    }

    return damage < 0 ? 0 : damage;
}