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

    static InitSkill* FindPlayerSkill(PLAYER_CLASS player_class, std::uint8_t skill_index);
    static InitSkill* FindAnimalSkill(std::uint8_t skill_index);
    static InitSkill* FindMonsterSkill(std::uint8_t skill_index);
};

class Skill
{
    const InitSkill* m_init;
    bango::utils::time::point m_last_use_time;
    std::uint8_t m_level;
    RESIST_TYPE resist_type;
protected:
    const Player& m_player;

public:
    Skill(const InitSkill* init, const Player& player, std::uint8_t level);

    std::uint8_t GetLevel() const { return m_level; }
    std::uint8_t GetIndex() const { return m_init->Index; }
    void SetLevel(std::uint8_t level) { m_level = level; }

    bango::utils::time::point GetRemainingCooldown() const { return m_last_use_time + std::chrono::milliseconds(m_init->CoolDown); }
    void UpdateCooldownTime() { m_last_use_time = bango::utils::time::now();}
    virtual void Execute(bango::network::packet& packet) {}
    virtual bool CanExecute(const Character& target) const;
    std::int32_t GetAttack() const { return 0; } // TODO: Implement
    RESIST_TYPE GetResist() const { return resist_type; }
    virtual bango::network::packet BuildCastPacket(id_t target_id = 0) const;
};

class PhysicalSkill : public Skill
{
    ATTACK_TYPE m_attack_type;
public:
    PhysicalSkill(const InitSkill* init, const Player& player, std::uint8_t level, ATTACK_TYPE attack_type)
        : Skill(init, player, level),
        m_attack_type(attack_type)
        {}

    ATTACK_TYPE GetAttackType() const { return m_attack_type; }
};

class MagicSkill : public Skill
{
public:
    MagicSkill(const InitSkill* init, const Player& player, std::uint8_t level)
        : Skill(init, player, level)
        {}
};

class Behead : public Skill
{
public:
    Behead(const InitSkill* init, const Player& player, std::uint8_t level)
        : Skill(init, player, level)
        {}
    virtual bool CanExecute(const Character& target) const override;
    virtual void Execute(bango::network::packet& packet) override;
};

class SkillManager
{
    const Player& m_player;
    std::unordered_map<std::uint8_t, std::unique_ptr<Skill>> m_skills;
public:
    SkillManager(const Player& player)
        : m_player(player)
        {}

    void Reset() { m_skills.clear(); }
    std::unique_ptr<Skill> CreateSkill(const InitSkill* init, std::uint8_t index, std::uint8_t level);
    bool Upgrade(std::uint8_t index, std::uint8_t level);
    bool Learn(const InitSkill* init, std::uint8_t index, std::uint8_t level = 1);
    bool Exists(std::uint8_t index) const;
    Skill* GetByIndex(std::uint8_t index) const;
};