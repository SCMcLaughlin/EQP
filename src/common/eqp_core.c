
#include "eqp_core.h"

void core_init(R(Core*) core, int sourceId, R(IpcBuffer*) logIpc)
{
    // Init the Basic object first
    basic_init(B(core), sourceId, logIpc);
    
    db_init(core, &core->database);
    db_thread_init(B(core), &core->dbThread);
}

void core_deinit(R(Core*) core)
{
    db_deinit(&core->database);
    db_thread_deinit(&core->dbThread);
    
    // Deinit the Basic object last
    basic_deinit(B(core));
}

void core_start_threads(R(Core*) core)
{
    db_thread_start(B(core), &core->dbThread);
}

void core_send_stop_signal_to_threads(R(Core*) core)
{
    thread_send_stop_signal(B(core), T(&core->dbThread));
}

void core_wait_for_threads_to_stop(R(Core*) core)
{
    thread_wait_until_stopped(T(&core->dbThread));
}
