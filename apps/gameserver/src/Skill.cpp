#include "Skill.h"

#include <utility>

#include "Player.h"
#include "World.h"

#include <inix.h>

#include "spdlog/spdlog.h"

using namespace bango::network;
using namespace bango::utils;

unsigned int InitSkill::index() const
{
    if (Class >= 0)
        return Class * 100 + Index;
    return (std::abs(Class) + MAX_CHARACTER) * 100 + Index;
}

InitSkill* InitSkill::FindPlayerSkill(std::uint8_t player_class, std::uint8_t skill_index)
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
        case A_CLASS:           Class           = param.pop(); break;
        case A_INDEX:           Index           = param.pop(); break;
        case A_REDISTRIBUTE:    Redistribute    = param.pop(); break;
        case A_LIMIT:
        {
            LevelLimit          = param.pop();
            Job                 = 1 << (uint32_t)param.pop();
            RequiredSkillIndex  = param.pop();
            RequiredSkillGrade  = param.pop();
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

Skill* SkillManager::GetByIndex(std::uint8_t index) const
{
    auto it = m_skills.find(index);
    return it == m_skills.end() ? nullptr : it->second.get();
}

#define MAKE_SKILL_TYPE(type) std::make_unique<type>(init, m_player, level); 

std::unique_ptr<Skill> SkillManager::CreateSkill(std::uint8_t index, std::uint8_t level)
{
    const auto* init = InitSkill::FindPlayerSkill(m_player.GetClass(), index);
    if (!init)
        return nullptr;

    if (index == 1)
        return MAKE_SKILL_TYPE(Behead);

    switch (m_player.GetClass())
    {
    case PC_KNIGHT:
        switch (index)
        {
        case SK_WEAPON_MASTERY: return MAKE_SKILL_TYPE(WeaponMastery);
        case SK_LIGHTNING_SLASH: return MAKE_SKILL_TYPE(LightningSlash);
        default:
            break;
        }
        break;
    case PC_ARCHER:
        switch (index)
        {
        case SA_STAGGERING_BLOW: return MAKE_SKILL_TYPE(StaggeringBlow);
        default:
            break;
        }
        break;
    case PC_MAGE:
        switch (index)
        {
        case SM_FIRE_MAGIC: return MAKE_SKILL_TYPE(LightningMagic);
        case SM_ICE_MAGIC: return MAKE_SKILL_TYPE(IceMagic);
        case SM_LIGHTNING_MAGIC: return MAKE_SKILL_TYPE(FireMagic);
        default:
            break;
        }
        break;
    case PC_THIEF:
        break;
    case PC_SHAMAN:
        break;
    }

    // default
    return MAKE_SKILL_TYPE(Skill);
}

Skill* SkillManager::Add(std::uint8_t index, std::uint8_t level)
{
    auto skill = CreateSkill(index, level);
    if (!skill)
        return nullptr;
    auto [it, success] = m_skills.insert(std::make_pair(index, std::move(skill)));
    if (success)
        return it->second.get();
    return nullptr;
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

bool Skill::CanLearn() const
{
    if (GetLevel() >= m_init->MaxLevel)
        return false;
    return true;
}

void Skill::Execute(bango::network::packet& p)
{
    spdlog::info("This skill is not usable.");
}

bool Behead::CanExecute(const Character& target) const
{
    return target.IsGState(CGS_KNEE)
        && !target.IsGState(CGS_KO)
        && target.GetType() == Character::MONSTER;
}

bool SingleTargetSkill::CanExecute(const Character& target) const
{
    return Skill::CanExecute(target) && GetCaster().GetID() != target.GetID();
}


void Behead::ExecuteSpecificBehavior(Character& target)
{
    spdlog::info("Want to behead!");

    target.AddGState(CGS_KO);
    target.SubGState(CGS_KNEE);
    target.WriteInSight(packet(S2C_ACTION, "db", target.GetID(), AT_DIE));

    std::uint8_t is_damage = 0;
    GetCaster().WriteInSight(packet(S2C_SKILL, "bddbb",
        GetIndex(),
        GetCaster().GetID(),
        target.GetID(),
        is_damage,
        GetLevel()));

    // TODO: Restore caster HP/MP in a threadsafe manner
}

void PhysicalSkill::ExecuteSpecificBehavior(Character& target)
{
    std::uint8_t is_damage = (std::uint8_t)GetCaster().CheckHit(target);
    if (!is_damage)
    {
        GetCaster().WriteInSight(packet(S2C_SKILL, "bddbbwwb",
            GetIndex(),
            GetCaster().GetID(),
            target.GetID(),
            0,
            GetLevel(), 
            0,
            0,
            target.GetType()));
        return;
    }
    std::int16_t damage = GetAttack(/*TODO: revise tick*/);
    if ((uint64_t)damage > target.GetCurHP())
        damage = target.GetCurHP();

    target.ReceiveDamage(GetCaster().GetID(), damage);

    if (target.GetCurHP() <= 0)
        target.Die();

    std::int64_t explosive_blow = 0;
    GetCaster().WriteInSight(packet(S2C_SKILL, "bddbbwwb",
        GetIndex(),
        GetCaster().GetID(),
        target.GetID(),
        is_damage,
        GetLevel(),
        damage,
        explosive_blow,
        target.GetType()));
}

void MagicSkill::ExecuteSpecificBehavior(Character& target)
{
    std::int16_t damage = GetMagic(/*TODO: revise tick*/);
    if ((uint64_t)damage > target.GetCurHP())
        damage = target.GetCurHP();

    target.ReceiveDamage(GetCaster().GetID(), damage);

    if (target.GetCurHP() <= 0)
        target.Die();


    std::uint8_t is_damage = damage > 0 ? 1 : 0;
    std::int64_t explosive_blow = 0;

    GetCaster().WriteInSight(packet(S2C_SKILL, "bddbbwwb",
        GetIndex(),
        GetCaster().GetID(),
        target.GetID(),
        is_damage,
        GetLevel(),
        damage,
        explosive_blow,
        target.GetType()));
}

void SingleTargetSkill::Execute(bango::network::packet& p)
{
    auto kind = p.pop<std::uint8_t>();
    auto id = p.pop<Character::id_t>();

    auto query_kind = kind == CK_PLAYER ? WorldMap::QK_PLAYER : WorldMap::QK_MONSTER;
    World::ForCharacterInMap(GetCaster().GetMap(), query_kind, id, [&](Character& target)
    {
        auto lock = target.Lock();

        if (!CanExecute(target))
            return;

        ExecuteSpecificBehavior(target);
    });
}

std::uint16_t StaggeringBlow::GetAttack() const
{
    std::uint16_t max_add_attack = 70 * GetLevel() + 200;
    std::uint16_t add_attack = 7 * GetLevel() * GetCaster().GetDexterity() / 4 / 2 + 50;


    return std::min(add_attack, max_add_attack) + PhysicalSkill::GetAttack();
}

void WeaponMastery::OnApply(std::uint8_t previous_level)
{
    // OTP
    int previous_otp = previous_level / 2;
    int current_otp = GetLevel() / 2;
    GetCaster().UpdatePropertyPoint(P_HIT, current_otp - previous_otp);

    // TODO: Implement Min/Max attack
}
std::uint16_t LightningSlash::GetAttack() const
{
    return GetCaster().GetAttack() + 8 * GetLevel();
}

//TODO: this 3 methods code is redundant
std::uint16_t LightningMagic::GetMagic() const
{
    std::uint16_t level_multiply = 3 * GetCaster().GetLevel() / 2;
    std::uint16_t min_add_magic = (level_multiply + 2 * GetLevel() * GetCaster().GetInteligence() / 16) / 2;
    std::uint16_t max_add_magic = min_add_magic; // TODO: GetPropertyPoint from lightning mastery

    std::uint16_t min_add_magic_limit = 12 * GetLevel() + 30;
    std::uint16_t max_add_magic_limit = 12 * GetLevel() + 180;

    min_add_magic = std::min(min_add_magic, min_add_magic_limit);
    max_add_magic = std::min(max_add_magic, max_add_magic_limit);

    std::uint16_t damage = bango::utils::random::between(min_add_magic, max_add_magic) + MagicSkill::GetMagic();

    return damage;
}

std::uint16_t FireMagic::GetMagic() const
{
  std::uint16_t level_multiply = 3 * GetCaster().GetLevel() / 2;
  std::uint16_t add_magic = (level_multiply + 3 * GetLevel() * GetCaster().GetInteligence() / 10) / 2;
  std::uint16_t max_add_magic = 21 * GetLevel() + 30;
  std::uint16_t damage = std::min(max_add_magic, add_magic) + MagicSkill::GetMagic() + 2 * GetLevel();

  return damage;
}

std::uint16_t IceMagic::GetMagic() const
{
  std::uint16_t level_multiply = 3 * GetCaster().GetLevel() / 2;
  std::uint16_t add_magic = (level_multiply + 3 * GetLevel() * GetCaster().GetInteligence() / 13) / 2;
  std::uint16_t max_add_magic = 16 * GetLevel() + 30;
  std::uint16_t damage = std::min(max_add_magic, add_magic) + MagicSkill::GetMagic();

  return damage;
}
