
#ifndef EQP_DEFINE_SHARE_MEM_H
#define EQP_DEFINE_SHARE_MEM_H

#include "define.h"

#ifdef EQP_LINUX
# include <unistd.h>
# include <sys/mman.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <fcntl.h>
#endif

#endif//EQP_DEFINE_SHARE_MEM_H
