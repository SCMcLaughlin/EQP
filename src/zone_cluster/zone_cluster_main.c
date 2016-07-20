
#include "zone_cluster.h"
#include "eqp_string.h"
#include "terminal.h"

int zone_cluster_main(ZC* zc, const char** argv)
{
    ExceptionScope exScope;
    int volatile ret = 0;
    int ex;
    
    exception_begin_try(B(zc), &exScope);
    
    switch ((ex = exception_try(B(zc))))
    {
    case Try:
        zc_init(zc, argv[1], argv[2], argv[3], argv[4], argv[5]);
        zc_main_loop(zc);
        break;
    
    case Finally:
        zc_deinit(zc);
        break;
    
    default:
        printf(TERM_RED "[zone_cluster_main] Unhandled exception (%i): %s\n" TERM_DEFAULT, ex, string_data(exception_get_message(B(zc))));
        exception_handled(B(zc));
        ret = ex;
        break;
    }
    
    exception_end_try_with_finally(B(zc));
    
    return ret;
}

int main(int argc, const char** argv)
{
    ZC zc;
    
    // argv[1] = path to zc ipc buffer
    // argv[2] = path to master ipc buffer
    // argv[3] = path to log-writer ipc buffer
    // argv[4] = id number for this zone cluster
    // argv[5] = port to listen for UDP packets on
    
    if (argc < 6)
    {
        printf("[zone_cluster_main] Not all required command line inputs provided (expected 5, got %i); aborting\n", argc - 1);
        return 1;
    }
    
    memset(&zc, 0, sizeof(ZC));
    
    basic_preinit(B(&zc));
    return zone_cluster_main(&zc, argv);
}
