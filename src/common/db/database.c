
#include "database.h"
#include "eqp_core.h"
#include "eqp_string.h"

Database* db_create(R(Core*) core)
{
    R(Database*) db = eqp_alloc_type(B(core), Database);
    db_init(core, db);
    return db;
}

void db_destroy(R(Database*) db)
{
    if (!db) return;
    db_deinit(db);
    free(db);
}

void db_init(R(Core*) core, R(Database*) db)
{
    db->core        = core;
    db->sqlite      = NULL;
    db->dbThread    = core_db_thread(core);
    db->dbPath      = NULL;
    db->nextQueryId = 0;
}

void db_deinit(R(Database*) db)
{
    sqlite3_close_v2(db->sqlite);
    
    if (db->dbPath)
    {
        free(db->dbPath);
        db->dbPath = NULL;
    }
}

static void db_create_from_schema(R(Database*) db, R(const char*) dbPath, R(const char*) schemaPath)
{
    R(FILE*) volatile schema;
    
    int rc = sqlite3_open_v2(
        dbPath,
        &db->sqlite,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_CREATE,
        NULL
    );
    
    if (rc != SQLITE_OK)
        exception_throw_format(B(db->core), ErrorDatabase, "[db_create_from_schema] Could not create database file '%s'\nReason: %s", dbPath, sqlite3_errstr(rc));
    
    if (!schemaPath)
        return;
    
    schema = fopen(schemaPath, "rb");
    
    if (schema)
    {
        ExceptionScope exScope;
        R(Core*) volatile core  = db->core;
        R(String*) volatile str = NULL;
        
        switch (exception_try(B(core), &exScope))
        {
        case Try:
            str = string_create_from_file(B(core), schema);
            
            log_format(B(core), LogInfo, "Database file '%s' did not exist, creating from schema file '%s'", dbPath, schemaPath);
            db_exec(db, string_data(str), "[db_create_from_schema] Error running schema file\nReason: %s");
            break;
        
        case Finally:
            fclose(schema);
        
            if (str)
                free(str);
            
            break;
            
        default: break;
        }
        
        exception_end_try_with_finally(B(core));
    }
}

void db_open(R(Database*) db, R(const char*) dbPath, R(const char*) schemaPath)
{
    int rc = sqlite3_open_v2(
        dbPath,
        &db->sqlite,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX,
        NULL
    );
    
    if (rc != SQLITE_OK)
    {
        if (rc != SQLITE_CANTOPEN)
            exception_throw_format(B(db->core), ErrorDatabase, "[db_load] Could not open database file '%s'\nReason: %s", dbPath, sqlite3_errstr(rc));
        
        db_create_from_schema(db, dbPath, schemaPath);
    }
    
    db->dbPath = string_create_from_cstr(B(db->core), dbPath, strlen(dbPath));
    log_format(B(db->core), LogInfo, "Database file '%s' opened and initialized", dbPath);
}

void db_prepare(R(Database*) db, R(Query*) query, R(const char*) sql, int len, QueryCallback callback)
{
    PerfTimer timer;
    sqlite3_stmt* stmt;
    int rc;
    
    perf_init(&timer);
    
    if (callback)
        query_set_callback(query, callback);
    
    rc = sqlite3_prepare_v2(db->sqlite, sql, len, &stmt, NULL);
    
    if (rc == SQLITE_OK)
    {
        query_set_db_and_stmt(query, db, stmt);
        log_format(B(db->core), LogSql, "Prepared query %u against database '%s' in %llu microseconds. SQL: \"%s\"",
            db->nextQueryId, string_data(db->dbPath), perf_microseconds(&timer), sql);
        return;
    }
    
    exception_throw_format(B(db->core), ErrorSql, "[db_prepare] Could not prepare query\nReason: %s\nSQL: %s", sqlite3_errstr(rc), sql);
}

void db_exec(R(Database*) db, R(const char*)sql, R(const char*) exceptionFormat)
{
    char* errmsg = NULL;
    int rc;

    rc = sqlite3_exec(db->sqlite, sql, NULL, NULL, &errmsg);
    
    if (errmsg)
    {
        ExceptionScope exScope;
        R(Basic*) volatile basic = B(db->core);
        
        switch (exception_try(basic, &exScope))
        {
        case Try:
            if (exceptionFormat)
                exception_throw_format(basic, ErrorSql, exceptionFormat, errmsg);
            
            exception_throw_format(basic, ErrorSql, "[db_exec] Could not complete query\nReason: %s", errmsg);
            break;
            
        case Finally:
            sqlite3_free(errmsg);
            break;
        
        default: break;
        }
        
        exception_end_try_with_finally(basic);
    }
    
    // Is it even possible to get this without errmsg being filled?
    if (rc != SQLITE_OK)
    {
        if (exceptionFormat)
            exception_throw_format(B(db->core), ErrorSql, exceptionFormat, sqlite3_errstr(rc));
        
        exception_throw_format(B(db->core), ErrorSql, "[db_exec] Could not complete query\nReason: %s", sqlite3_errstr(rc));
    }
}

void db_schedule(R(Database*) db, R(Query*) query)
{
    db_thread_schedule_query(B(db->core), db->dbThread, query);
}
