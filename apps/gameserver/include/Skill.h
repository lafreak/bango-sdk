#pragma once

#include <unordered_map>
#include <memory>

#include "Character.h"

#include <bango/processor/db.h>
#include <inix.h>
#include <bango/network/packet.h>

class InitSkill : public bango::processor::db_object<InitSkill>
{
    static constexpr std::uint8_t MAX_SKILL_INDEX = 84;
    enum Kind : int64_t { ANIMAL=1, MONSTER=2 };

public:
    unsigned int index() const;
    virtual void set(bango::processor::lisp::var param) override;

    std::int32_t Class = 0;
    std::uint32_t Index = 0;
    bool Redistribute = true;
    std::int32_t LevelLimit = 0;
    std::uint32_t Job = 0;
    std::uint32_t RequiredSkillIndex = 0;
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

    static InitSkill* FindPlayerSkill(PLAYER_CLASS player_class, std::uint8_t skill_index);
    static InitSkill* FindAnimalSkill(std::uint8_t skill_index);
    static InitSkill* FindMonsterSkill(std::uint8_t skill_index);
};

class Skill
{
    const InitSkill* m_init;
    std::uint32_t m_last_use;
    std::uint8_t m_level;

public:
    Skill(const InitSkill* init,const std::uint8_t skill_level);

    const std::uint8_t GetLevel() const { return m_level; }
    const std::uint8_t GetIndex() const { return m_init->Index; }
    void SetLevel(std::uint8_t level) { m_level = level; }
};

class SkillManager
{
public:
    std::unordered_map<std::uint8_t, std::unique_ptr<Skill>> m_skills;

    bool UpgradeSkill(const std::uint8_t skill_id, const std::uint8_t skill_level);
    bool LearnSkill(const InitSkill* skill_init,const std::uint8_t skill_id, const std::uint8_t skill_level = 1);
    bool HasSkill(const std::uint8_t skill_id) const;
    Skill* GetSkill(const std::uint8_t skill_id) const;  //pointers
};