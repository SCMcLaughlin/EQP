
#ifndef EQP_TRANSACTION_H
#define EQP_TRANSACTION_H

#include "define.h"
#include "query.h"

STRUCT_DECLARE(Database);
STRUCT_DECLARE(Transaction);

typedef void(*TransactionCallback)(Transaction* transaction);

STRUCT_DEFINE(Transaction)
{
    Database*           db;
    void*               userdata;
    TransactionCallback callback;
    int                 luaCallback;
    QueryCallback       queryCallback;
};

#define transaction_set_db(trans, database)         ((trans)->db = (database))
#define transaction_set_userdata(trans, ud)         ((trans)->userdata = (ud))
#define transaction_set_callback(trans, cb)         ((trans)->callback = (cb))
#define transaction_set_lua_callback(trans, index)  ((trans)->luaCallback = (index))
#define transaction_set_query_callback(trans, cb)   ((trans)->queryCallback = (cb))

#define transaction_db(trans)                   ((trans)->db)
#define transaction_userdata(trans)             ((trans)->userdata)
#define transaction_userdata_type(trans, type)  ((type*)transaction_userdata(trans))
#define transaction_callback(trans)             ((trans)->callback)
#define transaction_lua_callback(trans)         ((trans)->luaCallback)
#define transaction_query_callback(trans)       ((trans)->queryCallback)

#endif//EQP_TRANSACTION_H
