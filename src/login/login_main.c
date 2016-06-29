
#include "eqp_login.h"
#include "eqp_string.h"
#include "terminal.h"

int login_main(Login* login, const char** argv)
{
    ExceptionScope exScope;
    int volatile ret = 0;
    int ex;
    
    switch ((ex = exception_try(B(login), &exScope)))
    {
    case Try:
        login_init(login, argv[1], argv[2], argv[3]);
        login_main_loop(login);
        break;
    
    case Finally:
        login_deinit(login);
        break;
    
    default:
        printf(TERM_RED "[login_main] Unhandled exception (%i): %s\n" TERM_DEFAULT, ex, string_data(exception_get_message(B(login))));
        exception_handled(B(login));
        ret = ex;
        break;
    }
    
    exception_end_try_with_finally(B(login));
    
    return ret;
}

int main(int argc, const char** argv)
{
    Login login;
    
    // argv[1] = path to login ipc buffer
    // argv[2] = path to master ipc buffer
    // argv[3] = path to log-writer ipc buffer
    
    if (argc < 4)
    {
        printf("[login_main] Not all IPC paths provided (expected 3, got %i); aborting\n", argc - 1);
        return 1;
    }
    
    memset(&login, 0, sizeof(Login));
    
    basic_preinit(B(&login));
    return login_main(&login, argv);
}
