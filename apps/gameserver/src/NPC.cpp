#include "NPC.h"

#include <memory>

#include "spdlog/spdlog.h"

NPC::NPC(const std::unique_ptr<InitNPC>& init)
    : Character(Character::NPC), m_init(init)
{
    spdlog::trace("NPC constructor id: {}", GetID());

    m_x =   init->X;
    m_y =   init->Y;
    m_z =   init->Z;
    m_map = init->Map;

    LookAt(init->DirX, init->DirY);
}

NPC::~NPC()
{
    spdlog::trace("NPC constructor id: {}", GetID());
}

void NPC::Tick()
{
}