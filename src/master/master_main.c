
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
    
    memset(&M, 0, sizeof(Master));
    
    basic_preinit(B(&M));
    return master_main(&M);
}
