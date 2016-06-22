
#ifndef EQP_CLIENT_H
#define EQP_CLIENT_H

#include "define.h"
#include "network_interface.h"
#include "mob.h"

STRUCT_DEFINE(Client)
{
    // Mob must be the first member of this struct
    Mob     mob;
};

#endif//EQP_CLIENT_H
