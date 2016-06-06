
#include "eqp_core.h"

void core_init(R(Core*) core, int sourceId, R(IpcBuffer*) logIpc)
{
    // Init the Basic object first
    basic_init(B(core), sourceId, logIpc);
    
    db_init(core, &core->database);
    db_thread_init(B(core), &core->dbThread);
    
    // Start db thread and open the db
    db_open(core_db(core), EQP_SQLITE_MAIN_DATABASE_PATH, EQP_SQLITE_MAIN_SCHEMA_PATH);
    db_thread_start(B(core), &core->dbThread);
}

void core_deinit(R(Core*) core)
{
    // Stop the DB thread and wait
    thread_send_stop_signal(B(core), T(&core->dbThread));
    thread_wait_until_stopped(T(&core->dbThread));
    
    // The DB thread tries to finish up any last pending queries before stopping,
    // so don't deinit the DB and DB thread until it finishes
    db_deinit(&core->database);
    db_thread_deinit(&core->dbThread);
    
    // Deinit the Basic object last
    basic_deinit(B(core));
}
