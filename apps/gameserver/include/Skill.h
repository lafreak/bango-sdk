#pragma once

#include <bango/processor/db.h>

struct InitSkill : public bango::processor::db_object<InitSkill>
{
    static constexpr std::int32_t CLASS_ANIMAL = -1;
    static constexpr std::int32_t CLASS_MONSTER = -2;

    std::int32_t Class = 0;
    std::uint32_t Index = 0;
    bool Redistribute = true;
    std::int32_t LevelLimit = 0;
    std::uint32_t Job = 0;
    std::uint32_t RequiredSkillId = 0;
    std::uint32_t RequiredSkillGrade = 0;
    std::uint32_t MaxLevel = 0;
    std::uint32_t MP = 0;
    std::uint32_t LastTime = 0;
    std::uint32_t CastTime = 0;
    std::uint32_t CoolDown = 0;
    // std::uint32_t Unknown = 0; // third argument of delay
    std::int32_t Value1 = 0;
    std::int32_t Value2 = 0;
    std::int32_t Rage = 0;

    static InitSkill* Find(std::int8_t entity_class, std::uint8_t skill_id);
    static InitSkill* FindAnimal(std::uint8_t skill_id);
    static InitSkill* FindMonster(std::uint8_t skill_id);
    unsigned int index() const;
    virtual void set(bango::processor::lisp::var param) override;

};