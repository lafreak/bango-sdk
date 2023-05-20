#pragma once

#include <unordered_map>
#include <memory>

#include "Character.h"

#include <bango/processor/db.h>
#include <bango/network/packet.h>
#include <bango/utils/time.h>
#include <inix.h>

class Player;

class InitSkill : public bango::processor::db_object<InitSkill>
{
    enum Kind : int64_t { ANIMAL=1, MONSTER=2 };

public:
    static constexpr std::uint8_t MAX_SKILL_INDEX = 84;

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

    static InitSkill* FindPlayerSkill(std::uint8_t player_class, std::uint8_t skill_index);
    static InitSkill* FindAnimalSkill(std::uint8_t skill_index);
    static InitSkill* FindMonsterSkill(std::uint8_t skill_index);
};

class Skill
{
    const InitSkill* m_init;

    Character& m_caster;
    std::uint8_t m_level;

public:
    Skill(const InitSkill* init, Character& caster, std::uint8_t level);

    const InitSkill& GetInit() const { return *this->m_init; }

    std::uint8_t GetLevel() const { return m_level; }
    std::uint8_t GetIndex() const { return m_init->Index; }
    Character& GetCaster() const { return m_caster; }

    void SetLevel(std::uint8_t level) { m_level = level; }

    virtual void Execute(bango::network::packet& packet);
    virtual bool CanExecute(const Character& target) const;
    virtual bool CanLearn() const;

    virtual void OnApply(std::uint8_t previous_level=0) {}
};

class SingleTargetSkill : public Skill
{
public:
    using Skill::Skill;

    void Execute(bango::network::packet& packet) override;
    bool CanExecute(const Character& target) const override;
    virtual void ExecuteSpecificBehavior(Character& target) = 0;
    virtual std::uint16_t GetAttack() const = 0;
};

class Behead : public SingleTargetSkill
{
public:
    using SingleTargetSkill::SingleTargetSkill;

    bool CanExecute(const Character& target) const override;
    void ExecuteSpecificBehavior(Character& target) override;
    std::uint16_t GetAttack() const override { return 0; }
};

class PhysicalSkill : public SingleTargetSkill
{
    using SingleTargetSkill::SingleTargetSkill;
    void ExecuteSpecificBehavior(Character& target) override;
};

class MagicSkill : public SingleTargetSkill
{
    //TODO: damage for mage skills and damage reduction based on resistances.
    using SingleTargetSkill::SingleTargetSkill;
    void ExecuteSpecificBehavior(Character& target) override;
};

class StaggeringBlow : public PhysicalSkill
{
public:
    using PhysicalSkill::PhysicalSkill;

    std::uint16_t GetAttack() const override;
    // TODO:
    //  - hostility
    //  - add otp
    //  - add crit rate
};

class WeaponMastery : public Skill
{
public:
    using Skill::Skill;

    void OnApply(std::uint8_t previous_level=0) override;
};

class LightningSlash : public PhysicalSkill
{
public:
    using PhysicalSkill::PhysicalSkill;

    std::uint16_t GetAttack() const override;
};

class LightningMagic : public MagicSkill
{
    using MagicSkill::MagicSkill;

    std::uint16_t GetAttack() const override;
};

class IceMagic : public MagicSkill
{
    using MagicSkill::MagicSkill;

    std::uint16_t GetAttack() const override;
    // TODO: Implement Ice Mastery interaction
};

class FireMagic : public MagicSkill
{
    using MagicSkill::MagicSkill;

    std::uint16_t GetAttack() const override;
};

class SkillManager
{
    Player& m_player;  // TODO: Similar manager is required for monsters as well
    std::unordered_map<std::uint8_t, std::unique_ptr<Skill>> m_skills;

    std::unique_ptr<Skill> CreateSkill(std::uint8_t index, std::uint8_t level);
public:
    SkillManager(Player& player) : m_player(player) {}

    Skill* Add(std::uint8_t index, std::uint8_t level);
    void Reset() { m_skills.clear(); }
    Skill* GetByIndex(std::uint8_t index) const;
};
