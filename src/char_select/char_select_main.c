
#include "eqp_char_select.h"
#include "eqp_string.h"
#include "terminal.h"

int char_select_main(CharSelect* charSelect, const char** argv)
{
    ExceptionScope exScope;
    int volatile ret = 0;
    int ex;
    
    exception_begin_try(B(charSelect), &exScope);
    
    switch ((ex = exception_try(B(charSelect))))
    {
    case Try:
        char_select_init(charSelect, argv[1], argv[2], argv[3]);
        char_select_start_login_server_connections(charSelect);
        char_select_main_loop(charSelect);
        break;
    
    case Finally:
        char_select_deinit(charSelect);
        break;
    
    default:
        printf(TERM_RED "[char_select_main] Unhandled exception (%i): %s\n" TERM_DEFAULT, ex, string_data(exception_get_message(B(charSelect))));
        exception_handled(B(charSelect));
        ret = ex;
        break;
    }
    
    exception_end_try_with_finally(B(charSelect));
    
    return ret;
}

int main(int argc, const char** argv)
{
    CharSelect charSelect;
    
    // argv[1] = path to char-select ipc buffer
    // argv[2] = path to master ipc buffer
    // argv[3] = path to log-writer ipc buffer
    
    if (argc < 4)
    {
        printf("[char_select_main] Not all IPC paths provided (expected 3, got %i); aborting\n", argc - 1);
        return 1;
    }
    
    memset(&charSelect, 0, sizeof(CharSelect));
    
    basic_preinit(B(&charSelect));
    return char_select_main(&charSelect, argv);
}
