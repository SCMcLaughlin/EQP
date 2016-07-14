
#include "query.h"
#include "database.h"
#include "eqp_core.h"
#include "eqp_string.h"

void query_init(Query* query)
{
    memset(query, 0, sizeof(Query));
    query->state = QUERY_NOT_YET_RUN;
}

void query_deinit(Query* query)
{
    sqlite3_finalize(query->stmt); // Safe if m_stmt is null
    query->stmt = NULL;
}

void query_set_db_and_stmt(Query* query, Database* db, sqlite3_stmt* stmt, uint32_t queryId)
{
    query->stmt         = stmt;
    query->database     = db;
    query->queryId      = queryId;
    query->timestamp    = clock_microseconds();
}

void query_update_last_insert_id(Query* query)
{
    sqlite3* sqlite     = db_get_sqlite(query->database);
    query->lastInsertId = sqlite3_last_insert_rowid(sqlite);
    query->affectedRows = sqlite3_changes(sqlite);
}

int query_execute_background(Query* query)
{
    int rc = sqlite3_step(query->stmt);
    
    switch (rc)
    {
    case SQLITE_BUSY:
        return false;
    
    case SQLITE_ROW:
        query_set_state(query, QUERY_HAS_RESULTS);
        break;
    
    case SQLITE_DONE:
        query_update_last_insert_id(query);
        break;
    
    default:
        exception_throw_format(B(db_get_core(query->database)), ErrorSql, "[query_execute_background] sqlite error %i: %s", rc, sqlite3_errstr(rc));
        break; // Unreachable
    }
    
    // Logging is thread safe
    log_format(B(db_get_core(query->database)), LogSql, "Executed query %i against database '%s' in %lu microseconds",
        query->queryId, db_get_path_cstr(query->database), clock_microseconds() - query->timestamp);
    return true;
}

void query_execute_synchronus(Query* query)
{
    for (;;)
    {
        if (query_execute_background(query))
            break;
    }
}

void query_execute_synchronus_insert_update(Query* query)
{
    query_execute_synchronus(query);
    query_reset(query);
}

void query_reset(Query* query)
{
    sqlite3_reset(query->stmt);
}

int query_select(Query* query)
{
    sqlite3_stmt* stmt;
    int rc;
    
    // The way we have things set up (to execute queries in the background) means that
    // when we call this the first time, we will already have the first row of results
    // queued up; need to special-case it
    if (query->state == QUERY_HAS_RESULTS)
    {
        query->state = SQLITE_OK;
        return true;
    }
    
    stmt = query->stmt;
    
    do
    {
        rc = sqlite3_step(stmt);
    }
    while (rc == SQLITE_BUSY); // Shouldn't happen now
    
    switch (rc)
    {
    case SQLITE_ROW:
        return true;
    
    case SQLITE_DONE:
        sqlite3_reset(stmt);
        break;
    
    default:
        exception_throw_format(B(db_get_core(query->database)), ErrorSql, "[query_select] %s", sqlite3_errstr(rc));
        break; // Unreachable
    }
    
    return false;
}

void query_bind_int(Query* query, int col, int val)
{
    int rc = sqlite3_bind_int(query->stmt, col, val);
    
    if (rc != SQLITE_OK)
        exception_throw_format(B(db_get_core(query->database)), ErrorSql, "[query_bind_int] %s", sqlite3_errstr(rc));
}

void query_bind_int64(Query* query, int col, int64_t val)
{
    int rc = sqlite3_bind_int64(query->stmt, col, val);
    
    if (rc != SQLITE_OK)
        exception_throw_format(B(db_get_core(query->database)), ErrorSql, "[query_bind_int64] %s", sqlite3_errstr(rc));
}

void query_bind_double(Query* query, int col, double val)
{
    int rc = sqlite3_bind_double(query->stmt, col, val);
    
    if (rc != SQLITE_OK)
        exception_throw_format(B(db_get_core(query->database)), ErrorSql, "[query_bind_double] %s", sqlite3_errstr(rc));
}

static void query_do_bind_string(Query* query, int col, const char* str, int len, void (*type)(void*))
{
    int rc = sqlite3_bind_text(query->stmt, col, str, len, type);
    
    if (rc != SQLITE_OK)
        exception_throw_format(B(db_get_core(query->database)), ErrorSql, "[query_do_bind_string] %s", sqlite3_errstr(rc));
}

void query_bind_string(Query* query, int col, const char* str, int len)
{
    query_do_bind_string(query, col, str, len, SQLITE_TRANSIENT);
}

void query_bind_string_no_copy(Query* query, int col, const char* str, int len)
{
    query_do_bind_string(query, col, str, len, SQLITE_STATIC);
}

static void query_do_bind_blob(Query* query, int col, const void* data, uint32_t len, void (*type)(void*))
{
    int rc = sqlite3_bind_blob(query->stmt, col, data, len, type);
    
    if (rc != SQLITE_OK)
        exception_throw_format(B(db_get_core(query->database)), ErrorSql, "[query_do_bind_blob] %s", sqlite3_errstr(rc));
}

void query_bind_blob(Query* query, int col, const void* data, uint32_t len)
{
    query_do_bind_blob(query, col, data, len, SQLITE_TRANSIENT);
}

void query_bind_blob_no_copy(Query* query, int col, const void* data, uint32_t len)
{
    query_do_bind_blob(query, col, data, len, SQLITE_STATIC);
}

int query_get_int(Query* query, int col)
{
    return sqlite3_column_int(query->stmt, col - 1);
}

int64_t query_get_int64(Query* query, int col)
{
    return sqlite3_column_int64(query->stmt, col - 1);
}

double query_get_double(Query* query, int col)
{
    return sqlite3_column_double(query->stmt, col - 1);
}

const char* query_get_string(Query* query, int col, uint32_t* len)
{
    sqlite3_stmt* stmt = query->stmt;
    col--;
    if (len)
        *len = sqlite3_column_bytes(stmt, col);
    return (const char*)sqlite3_column_text(stmt, col);
}

const byte* query_get_blob(Query* query, int col, uint32_t* len)
{
    sqlite3_stmt* stmt = query->stmt;
    col--;
    if (len)
        *len = sqlite3_column_bytes(stmt, col);
    return (const byte*)sqlite3_column_blob(stmt, col);
}

int query_is_null(Query* query, int col)
{
    col--;
    return (sqlite3_column_type(query->stmt, col) == SQLITE_NULL);
}
