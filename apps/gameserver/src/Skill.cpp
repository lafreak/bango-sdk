#include "Skill.h"

#include "spdlog/spdlog.h"

#include <inix.h>


unsigned int InitSkill::index() const
{
    return Index;
}

InitSkill* InitSkill::FindPlayerSkill(PLAYER_CLASS player_class, std::uint8_t skill_index)
{
    if (player_class < 0 || player_class >= MAX_CHARACTER)
    {
        spdlog::warn("Cannot find player skill for unknown class: {}", player_class);
        return nullptr;
    }

    if (skill_index > MAX_SKILL_INDEX)
        return nullptr;

    unsigned int index = player_class * 100 + skill_index;
    const auto& it = DB().find(index);

    return it == DB().end() ? nullptr : it->second.get();
}

InitSkill* InitSkill::FindAnimalSkill(std::uint8_t skill_index)
{
    if (skill_index > MAX_SKILL_INDEX)
        return nullptr;

    unsigned int index = (MAX_CHARACTER + Kind::ANIMAL) * 100 + skill_index;
    const auto& it = DB().find(index);

    return it == DB().end() ? nullptr : it->second.get();
}

InitSkill* InitSkill::FindMonsterSkill(std::uint8_t skill_index)
{
    if (skill_index > MAX_SKILL_INDEX)
        return nullptr;

    unsigned int index = (MAX_CHARACTER + Kind::MONSTER) * 100 + skill_index;
    const auto& it = DB().find(index);

    return it == DB().end() ? nullptr : it->second.get();
}

void InitSkill::set(bango::processor::lisp::var param)
{
    switch (FindAttribute(param.pop()))
    {
        case A_CLASS:        Class        = param.pop(); break;
        case A_INDEX:
        {
            Index = param.pop();
            if(Class >= 0) //Player skills
                Index = Class * 100 + Index;
            else           //Animal or Monster skills
                Index = (std::abs(Class) + MAX_CHARACTER) * 100 + Index;
            break;
        }
        case A_REDISTRIBUTE: Redistribute = param.pop(); break;
        case A_LIMIT:
        {
            LevelLimit = param.pop();
            Job        = param.pop();
            RequiredSkillIndex = param.pop();
            RequiredSkillGrade = param.pop();
            break;
        }
        case A_MAXLEVEL:     MaxLevel     = param.pop(); break;
        case A_MP:           MP           = param.pop(); break;
        case A_LASTTIME:     LastTime     = param.pop(); break;
        case A_DELAY:
        {
            CastTime = param.pop();
            CoolDown = param.pop();
            // Unknown  = param.pop();
            break;
        }
        case A_VALUE1:       Value1       = param.pop(); break;
        case A_VALUE2:       Value2       = param.pop(); break;
        case A_RAGE:         Rage         = param.pop(); break;

    }
}

bool SkillManager::Exists(std::uint8_t index) const
{
    return m_skills.find(index) != m_skills.end();
}


Skill* SkillManager::GetByIndex(std::uint8_t index) const
{
    return Exists(index) ? m_skills.at(index).get() : nullptr;
}

bool SkillManager::Upgrade(std::uint8_t index, std::uint8_t level)
{
    if(!Exists(index))
        return false;

    if(m_skills.at(index)->GetLevel() >= level)
        return false;

     m_skills.at(index)->SetLevel(level);
     return true;
}
bool SkillManager::Learn(const InitSkill* init,std::uint8_t index, std::uint8_t level)
{
    auto [_, success] = m_skills.insert({index, std::make_unique<Skill>(init, level)});
    return success;
}


Skill::Skill(const InitSkill* init,std::uint8_t level)
    : m_init(init),
    m_last_use(0), 
    m_level(level)
{
}
