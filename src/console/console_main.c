
#include "eqp_console.h"
#include "eqp_string.h"
#include "terminal.h"

int console_main(Console* console, int argc, const char** argv)
{
    ExceptionScope exScope;
    int volatile ret = 0;
    int ex;
    
    exception_begin_try(B(console), &exScope);
    
    switch ((ex = exception_try(B(console))))
    {
    case Try:
        console_init(console);
        if (console_send(console, argc, argv))
            console_recv(console);
        break;
    
    case Finally:
        console_deinit(console);
        break;
    
    case ErrorConsole:
        printf(TERM_RED "Error: %s\n" TERM_DEFAULT, string_data(exception_get_message(B(console))));
        exception_handled(B(console));
        break;
    
    default:
        printf(TERM_RED "[console_main] Unhandled exception type %i: %s\n" TERM_DEFAULT, ex, string_data(exception_get_message(B(console))));
        exception_handled(B(console));
        ret = ex;
        break;
    }
    
    exception_end_try_with_finally(B(console));
    
    return ret;
}

int main(int argc, const char** argv)
{
    Console console;
    const char* binPath;
    
    if (argc < 2)
    {
        printf("No command entered! Please provide a command to be executed, or use the -h option to list available commands\n");
        return 0;
    }
    
    binPath = getenv(EQP_PATH_ENV_VARIABLE);
    
    if (binPath)
#ifdef EQP_WINDOWS
        SetCurrentDirectoryA(binPath);
#else
        chdir(binPath);
#endif
    
    memset(&console, 0, sizeof(Console));
    
    basic_preinit(B(&console));
    return console_main(&console, argc, argv);
}
