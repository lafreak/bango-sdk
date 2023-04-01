#pragma once

#include <unordered_map>
#include <memory>

#include "Skill.h"

#include <bango/network/packet.h>

class SkillManager
{
    public:
    std::unordered_map<std::uint8_t, std::unique_ptr<Skill>> m_skills;

    void UpgradeSkill(const std::uint8_t skill_id);
    void InsertSkill(const InitSkill* skill_init,const std::uint8_t skill_id, const std::uint8_t skill_level = 1);
    bool HasSkill(const std::uint8_t skill_id) const;
    Skill* GetSkill(const std::uint8_t skill_id) const;  //pointers
};