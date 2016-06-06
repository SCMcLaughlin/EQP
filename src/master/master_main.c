
#include "eqp_master.h"
#include "eqp_string.h"
#include "terminal.h"

int master_main(R(Master*) M)
{
    ExceptionScope exScope;
    int volatile ret = 0;
    int ex;
    
    switch ((ex = exception_try(B(M), &exScope)))
    {
    case Try:
        master_init(M);
        master_start_ipc_thread(M);
        // Start core child processes
        master_start_log_writer(M);
        //master_start_login(M);
        master_main_loop(M);
        break;
    
    case Finally:
        master_deinit(M);
        break;
    
    default:
        printf(TERM_RED "[master_main] Unhandled exception (%i): %s\n" TERM_DEFAULT, ex, string_data(exception_get_message(B(M))));
        exception_handled(B(M));
        ret = ex;
        break;
    }
    
    exception_end_try_with_finally(B(M));
    
    return ret;
}

int main(int argc, const char** argv)
{
    Master M;
    
    (void)argc;
    (void)argv;
    
    basic_preinit(B(&M));
    return master_main(&M);
}

#if 0
#include "eqp_master.h"
#include "eqp_string.h"
#include "eqp_thread.h"

void e(R(Core*) core);
void f(R(Core*) core);
void g(R(Core*) core);

#include "ipc_buffer.h"
#include "share_mem.h"

IpcBuffer gIpc;

STRUCT_DEFINE(NumThread)
{
    Thread  thread;
    int     num;
};

static void thread_proc(R(Thread*) t)
{
    /*for (;;)
    {
        thread_wait(t);
        
        if (thread_should_stop(t))
            return;
        
        printf(G(t), "Thread proc! %p\n", t);
    }*/
    
    R(NumThread*) nt    = (NumThread*)t;
    int num             = nt->num;
    uint32_t i;
    int wrote = 0;
    uint64_t time = clock_microseconds();
    
    for (i = 0; i < 10000; i++)
    {
        if (ipc_buffer_write(B(t), &gIpc, ServerOpNone, num, 0, NULL))
            wrote++;
        //clock_sleep_milliseconds(10);
    }
    
    printf("wrote %i in %lu\n", wrote, clock_microseconds() - time);
}

static void open_db(Master* M)
{
    db_open(core_db(C(M)), EQP_SQLITE_MAIN_DATABASE_PATH, EQP_SQLITE_MAIN_SCHEMA_PATH);
}

static void try_query(Master* M, const char* sql)
{
    ExceptionScope exScope;
    Query query;
    
    query_init(&query);
    
    switch (exception_try(B(M), &exScope))
    {
    case Try:
        db_prepare(core_db(C(M)), &query, sql, -1, NULL);
        db_schedule(core_db(C(M)), &query);
        break;
    
    case Finally:
        query_deinit(&query);
        break;
    
    default:
        printf("Exception in try_query: %s\n", string_data(exception_get_message(B(M))));
        exception_handled(B(M));
        break;
    }
    
    exception_end_try_with_finally(B(M));
}

void run(R(Master*) master)
{
    ExceptionScope exScope;
    //uint32_t volatile i;
    //NumThread numThreads[20];
    //uint32_t time = clock_milliseconds();
    
    switch (exception_try(B(master), &exScope))
    {
    case Try:
        master_init(master);
        master_start_log_writer(master);
        
        core_start_threads(C(master));
    
        ipc_buffer_init(B(master), &gIpc);
        
        //thread_start_and_detach(B(&master), &thread, thread_proc);
        
        open_db(master);
        try_query(master, "INSERT INTO local_login VALUES (random(), 2, 3)");
        try_query(master, "INSERT INTO local_login VALUES (random(), 2, 3)");
        
        //thread_trigger(B(&master), &thread);
        /*for (i = 0; i < 2; i++)
        {
            numThreads[i].num = (int)i;
            thread_start_and_detach(B(master), T(&numThreads[i]), thread_proc);
        }
        
        for (;;)
        {
            IpcPacket packet;
            
            if ((clock_milliseconds() - time) >= 20000)
                break;
            
            if (ipc_buffer_read(B(master), &gIpc, &packet))
            {
                printf("%i\n", packet.sourceId);
                ipc_packet_deinit(&packet);
            }
            else
            {
                clock_sleep_milliseconds(2);
            }
        }*/
        
        //e(C(master));
        break;
    
    case Finally:
        //printf("finally\n");
        clock_sleep_milliseconds(5250);
        //thread_send_stop_signal(B(&master), &thread);
        //thread_wait_until_stopped(&thread);
        
        core_send_stop_signal_to_threads(C(master));
    
        /*for (i = 0; i < 2; i++)
        {
            thread_send_stop_signal(B(master), T(&numThreads[i]));
            thread_wait_until_stopped(T(&numThreads[i]));
        }*/
    
        core_wait_for_threads_to_stop(C(master));
        break;
    
    default:
        printf("Uncaught exception: %s\n", string_data(exception_get_message(B(master))));
        exception_handled(B(master));
        break;
    }
    
    exception_end_try_with_finally(B(master));
}

int main(void)
{
    Master master;
    //ShmViewer shmViewer;
    //ShmCreator shmCreator;
    
    memset(&master, 0, sizeof(Master));
    
    basic_preinit(B(&master));
    
    //shm_viewer_init(&shmViewer);
    //share_mem_create(B(&master), &shmCreator, &shmViewer, "shm/eqp-test", sizeof(IpcBuffer));
    
    run(&master);
    master_deinit(&master);
    
    //share_mem_destroy(&shmCreator, &shmViewer);
    
    return 0;
}

void e(R(Core*) core)
{
    ExceptionScope exScope;
    R(String*) volatile str = NULL;
    
    switch (exception_try(B(core), &exScope))
    {
    case Try:
        str = string_create_from_cstr(B(core), "Hello CORE~~~!", sizeof("Hello CORE~~~!") - 1);
        f(core);
        printf("Done (%u) '%s' %p\n", string_get_length(str), string_get_data(str), str);
        break;
    
    case Finally:
        printf("e finally %p\n", str);
        if (str)
            free(str);
        break;
        
    default:
        log_format(B(core), LogInfo, "Unhandled exception! %s", string_get_data(exception_get_message(B(core))));
        exception_handled(B(core));
        break;
    }
    
    exception_end_try_with_finally(B(core));
}

void f(R(Core*) core)
{
    ExceptionScope exScope;
    int ret;
    
    ret = exception_try(B(core), &exScope);
    
    switch (ret)
    {
    case Try:
        printf("f code block\n");
        //exception_throw(core, ErrorBadAlloc);
        g(core);
        break;
    
    case Finally:
        printf("f finally block\n");
        break;
    
    default:
        printf("f exception: %i\n", ret);
        //exception_handled(core);
        break;
    }
    
    exception_end_try_with_finally(B(core));
}

void g(R(Core*) core)
{
    ExceptionScope exScope;
    int ret;
    
    ret = exception_try(B(core), &exScope);
    
    switch (ret)
    {
    case Try:
        printf("g code block\n");
        exception_throw(B(core), ErrorOutOfMemory);
        break;
    
    case Finally:
        printf("g finally block\n");
        exception_throw_format(B(core), 4, "Uh-oh! The truck have started to move! %p", core);
        break;
    
    default:
        printf("g exception: %i %s\n", ret, string_get_data(exception_get_message(B(core))));
        //exception_handled(core);
        break;
    }
    
    exception_end_try_with_finally(B(core));
}
#endif
