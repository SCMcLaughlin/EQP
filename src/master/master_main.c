
#include "eqp_master.h"
#include "eqp_string.h"
#include "terminal.h"

int master_main(Master* M)
{
    ExceptionScope exScope;
    int volatile ret = 0;
    int ex;
    
    exception_begin_try(B(M), &exScope);
    
    switch ((ex = exception_try(B(M))))
    {
    case Try:
        master_init(M);
        master_start_log_writer(M);
        master_start_login(M);
        master_start_char_select(M);
        // Start the IPC thread now to avoid needing to hold the lock while the above processes start
        master_start_ipc_thread(M);
        master_main_loop(M);
        clock_sleep_milliseconds(500);
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
    
    memset(&M, 0, sizeof(Master));
    
    basic_preinit(B(&M));
    return master_main(&M);
}
