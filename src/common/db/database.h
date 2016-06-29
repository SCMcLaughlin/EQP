
#ifndef EQP_DATABASE_H
#define EQP_DATABASE_H

#include "define.h"
#include "query.h"
#include "db_thread.h"
#include "exception.h"
#include "eqp_clock.h"
#include "eqp_log.h"
#include <sqlite3.h>
#include <stdatomic.h>

#define EQP_SQLITE_MAIN_DATABASE_PATH   "db/eqp.db"
#define EQP_SQLITE_MAIN_SCHEMA_PATH     "db/schema.sql"

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(String);

STRUCT_DEFINE(Database)
{
    Core*       core;
    sqlite3*    sqlite;
    DbThread*   dbThread;
    String*     dbPath;
    atomic_uint nextQueryId;
};

EQP_API Database*   db_create(Core* core);
EQP_API void        db_destroy(Database* db);

void                db_init(Core* core, Database* db);
void                db_deinit(Database* db);
EQP_API void        db_open(Database* db, const char* dbPath, const char* schemaPath);

EQP_API void        db_prepare(Database* db, Query* query, const char* sql, int len, QueryCallback callback);
#define             db_prepare_literal(db, query, sql, callback) db_prepare(db, query, sql, sizeof(sql) - 1, callback)
void                db_exec(Database* db, const char*sql, const char* exceptionFormat);

EQP_API void        db_schedule(Database* db, Query* query);

#define             db_get_core(db) ((db)->core)
#define             db_get_sqlite(db) ((db)->sqlite)
#define             db_get_path_cstr(db) (string_data((db)->dbPath))
#define             db_get_next_query_id(db) atomic_fetch_add(&(db)->nextQueryId, 1)

#endif//EQP_DATABASE_H
