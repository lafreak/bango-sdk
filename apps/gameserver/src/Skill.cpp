#include "Skill.h"

#include <utility>

#include "Player.h"

#include "spdlog/spdlog.h"

#include <inix.h>

using namespace bango::network;
using namespace bango::utils;

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
            Job        = 1 << (uint32_t)param.pop();
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

std::unique_ptr<Skill> SkillManager::CreateSkill(const InitSkill* init, std::uint8_t index, std::uint8_t level)
{
    if(index > InitSkill::MAX_SKILL_INDEX)
        std::runtime_error("Skill index is out of range!");

    if (index == 1)
        return std::make_unique<Behead>(init, m_player, level);
    
    // default
    return std::make_unique<Skill>(init, m_player, level);
}

bool SkillManager::Learn(const InitSkill* init, std::uint8_t index, std::uint8_t level)
{
    try
    {
        auto [_, success] = m_skills.insert({index, CreateSkill(init, index, level)});
        return success;
    }
    catch(const std::exception& e)
    {
        spdlog::error("Failed to learn skill: {}", e.what());
        return false;
    }
}

Skill::Skill(const InitSkill* init, Character& caster, std::uint8_t level) :
    m_init(init), m_caster(caster), m_level(level)
{
}

bool Skill::CanExecute(const Character& target) const
{
    return
        m_caster.IsNormal() &&
        target.IsNormal() &&
        m_caster.GetMap() == target.GetMap();
}

void Skill::Execute(bango::network::packet& packet)
{
    spdlog::info("This skill is not usable.");
}

bool Behead::CanExecute(const Character& target) const
{
    return Skill::CanExecute(target)
        && target.IsGState(CGS_KNEE)
        && !target.IsGState(CGS_KO)
        && target.GetType() == Character::MONSTER;
}

void Behead::Execute(bango::network::packet& packet)
{
    spdlog::info("Want to behead!");
}

bool PhysicalSkill::CanExecute(const Character& target) const
{
    return Skill::CanExecute(target)
        && GetCaster().GetID() != target.GetID();
}

void PhysicalSkill::Execute(bango::network::packet& packet)
{
    spdlog::info("Want to use physical!");
}
