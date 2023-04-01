#include "SkillManager.h"

#include "spdlog/spdlog.h"

using namespace bango::network;

bool SkillManager::HasSkill(const std::uint8_t skill_id) const
{
    return m_skills.find(skill_id) != m_skills.end();
}


Skill* SkillManager::GetSkill(const std::uint8_t skill_id) const
{
    return HasSkill(skill_id) ? m_skills.at(skill_id).get() : nullptr;
}

void SkillManager::UpgradeSkill(const std::uint8_t skill_id)
{
    if(HasSkill(skill_id))
        m_skills.at(skill_id)->LevelUp();
}
void SkillManager::InsertSkill(const InitSkill* skill_init,const std::uint8_t skill_id, const std::uint8_t skill_level)
{
    auto it = m_skills.insert({skill_id, std::make_unique<Skill>(skill_init, skill_level)});
}
