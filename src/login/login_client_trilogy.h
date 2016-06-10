
#ifndef EQP_LOGIN_CLIENT_TRILOGY_H
#define EQP_LOGIN_CLIENT_TRILOGY_H

#include "define.h"
#include "aligned.h"
#include "query.h"
#include "database.h"
#include "login_crypto.h"
#include "login_client.h"
#include "network_interface.h"
#include "protocol_handler.h"
#include "login_structs_trilogy.h"

#define EQP_LOGIN_TRILOGY_VERSION "8-09-2001 14:25"
#define EQP_LOGIN_TRILOGY_BANNER_MESSAGE "Welcome to EQP!" /* Make this runtime configurable? */

ENUM_DEFINE(LoginOpcodesTrilogy)
{
    TrilogyOp_Credentials   = 0x0001,
    TrilogyOp_Error         = 0x0002,
    TrilogyOp_Session       = 0x0004,
    TrilogyOp_Exit          = 0x0005,
    TrilogyOp_ServerList    = 0x0046,
    TrilogyOp_Banner        = 0x0052,
    TrilogyOp_Version       = 0x0059
};

ENUM_DEFINE(LoginClientTrilogyState)
{
    LoginClientTrilogy_BrandNew,
    LoginClientTrilogy_VersionSent,
    LoginClientTrilogy_ProcessingCredentials,
    LoginClientTrilogy_AcceptedCredentials
};

#endif//EQP_LOGIN_CLIENT_TRILOGY_H
