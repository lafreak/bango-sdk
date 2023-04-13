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


std::unique_ptr<Skill> SkillManager::CreateSkill(const InitSkill* init, std::uint8_t index, std::uint8_t level)
{
    if(index > InitSkill::MAX_SKILL_INDEX)
        std::runtime_error("Skill index is out of range!");

    switch(init->Class)
    {
        case PLAYER_CLASS::PC_KNIGHT:
        {
            switch(index)
            {
                case SK_BEHEAD:
                    return std::make_unique<Behead>(init, m_player, level);
                default:
                    return std::make_unique<PhysicalSkill>(init, m_player, level, ATT_MEELE);
            }
        }
        case PLAYER_CLASS::PC_MAGE:
        {
            switch(index)
            {
                case SM_BEHEAD:
                    return std::make_unique<Behead>(init, m_player, level);
                default:
                    return std::make_unique<MagicSkill>(init, m_player, level);
            }
        }
        case PLAYER_CLASS::PC_ARCHER:
        {
            switch(index)
            {
                case SA_BEHEAD:
                    return std::make_unique<Behead>(init, m_player, level);
                default:
                    return std::make_unique<PhysicalSkill>(init, m_player, level, ATT_RANGE);
            }
        }
        case PLAYER_CLASS::PC_THIEF:
        {
            switch(index)
            {
                case ST_BEHEAD:
                    return std::make_unique<Behead>(init, m_player, level);
                default:
                    return std::make_unique<PhysicalSkill>(init, m_player, level, ATT_MEELE);
            }
        }
        case PLAYER_CLASS::PC_SHAMAN:
        {
            switch(index)
            {
                case SS_BEHEAD:
                    return std::make_unique<Behead>(init, m_player, level);
                default:
                    return std::make_unique<MagicSkill>(init, m_player, level);
            }
        }
        default:
            throw std::runtime_error("Unknown player class while creating skill!");
    }
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


Skill::Skill(const InitSkill* init, Character& player,std::uint8_t level)
    : m_init(init),
    m_player(player),
    m_last_use_time{}, // TODO: change when cooldown protection is added
    m_level(level)
{

}

time::duration Skill::GetRemainingCooldown() const
{
    auto remaining = m_last_use_time + std::chrono::milliseconds(m_init->CoolDown);
    return remaining < time::now() ? time::duration::zero() : remaining - time::now();
}

bool Skill::CanExecute(const Character& target) const
{
    return m_player.IsNormal()
        && target.IsNormal()
        && m_player.GetMap() == target.GetMap()
        && !m_player.IsGState(CGS_ONTRANSFORM)
        //&& m_player.IsWState(WS_WEAPON) // TODO: add this check when WearState is added.
        && m_player.GetCurMP() >= m_init->MP
        && m_player.CanAttack(target);
}

packet Skill::BuildCastPacket(id_t target_id) const
{
    return packet(S2C_SKILL, "bddbb", GetIndex(), m_player.GetID(), target_id, std::uint8_t(0 /* b - unused for now */), GetLevel());
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

}