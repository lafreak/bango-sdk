#include "NPC.h"

#include <iostream>

NPC::~NPC()
{
    std::cout << "NPC ptr [" << (int*)this << "] destructor" << std::endl;
}

void NPC::Tick()
{
}