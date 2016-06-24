
#ifndef EQP_QUERY_H
#define EQP_QUERY_H

#include "define.h"
#include "eqp_clock.h"
#include "eqp_log.h"
#include <sqlite3.h>

#define QUERY_HAS_RESULTS -1
#define QUERY_NOT_YET_RUN -2

STRUCT_DECLARE(Core);
STRUCT_DECLARE(Database);
STRUCT_DECLARE(Query);

typedef void(*QueryCallback)(R(Query*) query);

STRUCT_DEFINE(Query)
{
    sqlite3_stmt*   stmt;
    Database*       database;
    
    union
    {
        int     state;
        int64_t lastInsertId;
    };
    
    union
    {
        void*   userdata;
        int64_t userInt;
    };
    
    QueryCallback   callback;
    uint32_t        queryId;
    int             affectedRows;
    uint64_t        timestamp;
};

void        query_init(R(Query*) query);
void        query_deinit(R(Query*) query);

void        query_set_db_and_stmt(R(Query*) query, R(Database*) db, R(sqlite3_stmt*) stmt);
#define     query_set_state(q, st) ((q)->state = (st))
#define     query_set_callback(q, cb) ((q)->callback = (cb))
void        query_update_last_insert_id(R(Query*) query);

int         query_execute_background(R(Query*) query);
void        query_execute_synchronus(R(Query*) query);

int         query_select(R(Query*) query);

void        query_bind_int(R(Query*) query, int col, int value);
void        query_bind_int64(R(Query*) query, int col, int64_t value);
void        query_bind_double(R(Query*) query, int col, double value);
void        query_bind_string(R(Query*) query, int col, R(const char*) str, int len);
void        query_bind_string_no_copy(R(Query*) query, int col, R(const char*) str, int len);
#define     query_bind_string_literal(q, col, str) query_bind_string_no_copy((q), (col), (str), sizeof(str) - 1)
void        query_bind_blob(R(Query*) query, int col, R(const void*) blob, uint32_t len);
void        query_bind_blob_no_copy(R(Query*) query, int col, R(const void*) blob, uint32_t len);

int         query_get_int(R(Query*) query, int col);
int64_t     query_get_int64(R(Query*) query, int col);
double      query_get_double(R(Query*) query, int col);
const char* query_get_string(R(Query*) query, int col, R(uint32_t*) len);
const byte* query_get_blob(R(Query*) query, int col, R(uint32_t*) len);
int         query_is_null(R(Query*) query, int col);

#define     query_execute_callback(q) if ((q)->callback) (q)->callback((q))
#define     query_has_callback(q) ((q)->callback ? true : false)

#define     query_get_db(q) ((q)->database)
#define     query_get_state(q) ((q)->state)
#define     query_get_last_insert_id(q) ((q)->lastInsertId)
#define     query_last_insert_id(q) query_get_last_insert_id(q)
#define     query_get_userdata(q) ((q)->userdata)
#define     query_userdata_type(q, type) ((type*)(q)->userdata)
#define     query_get_user_int(q) ((q)->userInt)
#define     query_get_id(q) ((q)->queryId)
#define     query_affected_rows(q) ((q)->affectedRows)

#define     query_set_userdata(q, ud) ((q)->userdata = (ud))

#endif//EQP_QUERY_H
