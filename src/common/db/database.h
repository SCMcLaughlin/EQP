
#ifndef EQP_DATABASE_H
#define EQP_DATABASE_H

#include "define.h"
#include "query.h"
#include "db_thread.h"
#include "exception.h"
#include "eqp_clock.h"
#include "eqp_log.h"
#include <sqlite3.h>

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
    uint32_t    nextQueryId;
};

EQP_API Database*   db_create(R(Core*) core);
EQP_API void        db_destroy(R(Database*) db);

void                db_init(R(Core*) core, R(Database*) db);
void                db_deinit(R(Database*) db);
EQP_API void        db_open(R(Database*) db, R(const char*) dbPath, R(const char*) schemaPath);

EQP_API void        db_prepare(R(Database*) db, R(Query*) query, R(const char*) sql, int len, QueryCallback callback);
#define             db_prepare_literal(db, query, sql, callback) db_prepare(db, query, sql, sizeof(sql) - 1, callback)
void                db_exec(R(Database*) db, R(const char*)sql, R(const char*) exceptionFormat);

EQP_API void        db_schedule(R(Database*) db, R(Query*) query);

#define             db_get_core(db) ((db)->core)
#define             db_get_sqlite(db) ((db)->sqlite)
#define             db_get_path_cstr(db) (string_data((db)->dbPath))
#define             db_get_next_query_id(db) (++(db)->nextQueryId)

#endif//EQP_DATABASE_H
