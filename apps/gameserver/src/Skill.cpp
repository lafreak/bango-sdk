#include "Skill.h"

#include "spdlog/spdlog.h"

#include <inix.h>

unsigned int InitSkill::index() const
{
    return Index;
}

InitSkill* InitSkill::Find(std::int8_t entity_class, std::uint8_t skill_id)
{
    if(entity_class >= MAX_CHARACTER)
    {
        spdlog::warn("Invalid player class: {}", entity_class);
        return 0;
    }
    std::uint32_t index = 0;
    if(entity_class >= 0)
        index = (entity_class * 100) + skill_id;
    else
        index = ((std::abs(entity_class) + MAX_CHARACTER) * 100) + skill_id;

    const auto it = DB().find(index);
    if (it != DB().end())
        return it->second.get();
    
    return nullptr;
}
InitSkill* InitSkill::FindAnimal(std::uint8_t skill_id)
{
    return Find(CLASS_ANIMAL, skill_id);
}
InitSkill* InitSkill::FindMonster(std::uint8_t skill_id)
{
    return Find(CLASS_MONSTER, skill_id);
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
                Index = (Class * 100) + Index;
            else           //Animal or Monster skills
                Index = ((std::abs(Class) + MAX_CHARACTER) * 100) + Index;
            break;
        }
        case A_REDISTRIBUTE: Redistribute = param.pop(); break;
        case A_LIMIT:
        {
            LevelLimit = param.pop();
            Job        = param.pop();
            RequiredSkillId = param.pop();
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
