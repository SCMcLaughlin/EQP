
#include "map_gen.h"
#include "eqp_string.h"
#include "terminal.h"

static int map_gen_main(MapGen* map, int argc, const char** argv)
{
    ExceptionScope exScope;
    int volatile ret = 0;
    int ex;
    
    exception_begin_try(B(map), &exScope);
    
    switch ((ex = exception_try(B(map))))
    {
    case Try:
        map_gen_init(map);
        if (argc >= 3)
            map_gen_read_single_zone(map, argv[1], argv[2]);
        else
            map_gen_read_all_zones(map, argv[1]);
        break;
    
    case Finally:
        map_gen_deinit(map);
        basic_deinit(B(map));
        break;
    
    default:
        printf(TERM_RED "[map_gen_main] Unhandled exception (%i): %s\n" TERM_DEFAULT, ex, string_data(exception_get_message(B(map))));
        exception_handled(B(map));
        ret = ex;
        break;
    }
    
    exception_end_try_with_finally(B(map));
    
    return ret;
}

int main(int argc, const char** argv)
{
    MapGen map;
    
    if (argc < 2)
    {
        printf("No path provided, aborting\n");
        return 1;
    }
    
    memset(&map, 0, sizeof(map));
    
    basic_preinit(B(&map));
    return map_gen_main(&map, argc, argv);
}
