
#include "define.h"
#include "log_writer.h"

int log_main(LogWriter* logWriter, const char* ipcPath)
{
    ExceptionScope exScope;
    int volatile ret = 0;
    int ex;
    
    switch ((ex = exception_try(B(logWriter), &exScope)))
    {
    case Try:
        log_writer_init(logWriter, ipcPath);
        log_writer_main_loop(logWriter);
        break;
    
    case Finally:
        log_writer_deinit(logWriter);
        break;
    
    default:
        printf("[log_main] Unhandled exception (%i): %s\n", ex, string_data(exception_get_message(B(logWriter))));
        exception_handled(B(logWriter));
        ret = ex;
        break;
    }
    
    exception_end_try_with_finally(B(logWriter));
    
    return ret;
}

int main(int argc, const char** argv)
{
    LogWriter logWriter;
    
    // argv[1] = path to shared ipc buffer
    
    if (argc < 2)
    {
        printf("[log_main] No IPC path provided, aborting\n");
        return 1;
    }
    
    basic_preinit(B(&logWriter));
    return log_main(&logWriter, argv[1]);
}
