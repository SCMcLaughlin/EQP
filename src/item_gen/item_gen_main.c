
#include "item_gen.h"
#include "eqp_string.h"
#include "terminal.h"

static int item_gen_main(ItemGen* gen, int argc, const char** argv)
{
    ExceptionScope exScope;
    int volatile ret = 0;
    int ex;
    
    (void)argc;
    (void)argv;
    
    exception_begin_try(B(gen), &exScope);
    
    switch ((ex = exception_try(B(gen))))
    {
    case Try:
        item_gen_init(gen);
        item_gen_scan_and_generate(gen);
        break;
    
    case Finally:
        item_gen_deinit(gen);
        break;
    
    default:
        printf(TERM_RED "[item_gen_main] Unhandled exception (%i): %s\n" TERM_DEFAULT, ex, string_data(exception_get_message(B(gen))));
        exception_handled(B(gen));
        ret = ex;
        break;
    }
    
    exception_end_try_with_finally(B(gen));
    
    return ret;
}

int main(int argc, const char** argv)
{
    ItemGen gen;
    
    memset(&gen, 0, sizeof(gen));
    
    basic_preinit(B(&gen));
    return item_gen_main(&gen, argc, argv);
}
