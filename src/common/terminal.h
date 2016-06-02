
#ifndef _EQP_TERMINAL_H_
#define _EQP_TERMINAL_H_

#ifndef EQP_NO_TERMINAL_COLORS

# define TERM_DARK_RED      "\033[0;31m"
# define TERM_RED           "\033[1;31m"
# define TERM_DARK_GREEN    "\033[0;32m"
# define TERM_GREEN         "\033[1;32m"
# define TERM_DARK_YELLOW   "\033[0;33m"
# define TERM_YELLOW        "\033[1;33m"
# define TERM_DARK_BLUE     "\033[0;34m"
# define TERM_BLUE          "\033[1;34m"
# define TERM_DARK_MAGENTA  "\033[0;35m"
# define TERM_MAGENTA       "\033[1;35m"
# define TERM_DARK_CYAN     "\033[0;36m"
# define TERM_CYAN          "\033[1;36m"
# define TERM_DEFAULT       "\033[0m"

#else

# define TERM_DARK_RED      ""
# define TERM_RED           ""
# define TERM_DARK_GREEN    ""
# define TERM_GREEN         ""
# define TERM_DARK_YELLOW   ""
# define TERM_YELLOW        ""
# define TERM_DARK_BLUE     ""
# define TERM_BLUE          ""
# define TERM_DARK_MAGENTA  ""
# define TERM_MAGENTA       ""
# define TERM_DARK_CYAN     ""
# define TERM_CYAN          ""
# define TERM_DEFAULT       ""

#endif

#endif//_EQP_TERMINAL_H_
