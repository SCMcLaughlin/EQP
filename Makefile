
CC= clang

CFLAG= 
COPT= -O2 -fomit-frame-pointer -std=gnu11
CWARN= -Wall -Wextra -Wredundant-decls
CWARNIGNORE= -Wno-unused-result -Wno-strict-aliasing
CINCLUDE=
CDEF=

#ifdef debug
CFLAG+= -O0 -g -Wno-format -fno-omit-frame-pointer
BUILDTYPE= debug
#else
#CFLAG+= -DNDEBUG
#BUILDTYPE= release
#endif

DIRBIN= bin/

##############################################################################
# Common
##############################################################################
DIRCOMMON= src/common/
BCOMMON= build/$(BUILDTYPE)/common/
_OCOMMON= \
 eqp_basic.o eqp_core.o eqp_alloc.o bit.o exception.o eqp_thread.o file.o \
 random.o
_HCOMMON= define.h netcode.h \
 eqp_basic.h eqp_core.h eqp_alloc.h bit.h exception.h eqp_thread.h file.h \
 random.h
OCOMMON= $(patsubst %,$(BCOMMON)%,$(_OCOMMON))
HCOMMON= $(patsubst %,$(DIRCOMMON)%,$(_HCOMMON))

CINCLUDE+= -I$(DIRCOMMON)
OCOMMON_ALL= $(OCOMMON)
HCOMMON_ALL= $(HCOMMON)

##############################################################################
# Common / Container
##############################################################################
DIRCOMMON_CONTAINER= $(DIRCOMMON)container/
BCOMMON_CONTAINER= $(BCOMMON)container/
_OCOMMON_CONTAINER= eqp_string.o eqp_array.o
_HCOMMON_CONTAINER= eqp_string.h eqp_array.h
OCOMMON_CONTAINER= $(patsubst %,$(BCOMMON_CONTAINER)%,$(_OCOMMON_CONTAINER))
HCOMMON_CONTAINER= $(patsubst %,$(DIRCOMMON_CONTAINER)%,$(_HCOMMON_CONTAINER))

CINCLUDE+= -I$(DIRCOMMON_CONTAINER)
OCOMMON_ALL+= $(OCOMMON_CONTAINER)
HCOMMON_ALL+= $(HCOMMON_CONTAINER)

##############################################################################
# Common / DB
##############################################################################
DIRCOMMON_DB= $(DIRCOMMON)db/
BCOMMON_DB= $(BCOMMON)db/
_OCOMMON_DB= database.o db_thread.o query.o
_HCOMMON_DB= database.h db_thread.h query.h
OCOMMON_DB= $(patsubst %,$(BCOMMON_DB)%,$(_OCOMMON_DB))
HCOMMON_DB= $(patsubst %,$(DIRCOMMON_DB)%,$(_HCOMMON_DB))

CINCLUDE+= -I$(DIRCOMMON_DB)
OCOMMON_ALL+= $(OCOMMON_DB)
HCOMMON_ALL+= $(HCOMMON_DB)

##############################################################################
# Common / Log
##############################################################################
DIRCOMMON_LOG= $(DIRCOMMON)log/
BCOMMON_LOG= $(BCOMMON)log/
_OCOMMON_LOG= eqp_log.o
_HCOMMON_LOG= eqp_log.h
OCOMMON_LOG= $(patsubst %,$(BCOMMON_LOG)%,$(_OCOMMON_LOG))
HCOMMON_LOG= $(patsubst %,$(DIRCOMMON_LOG)%,$(_HCOMMON_LOG))

CINCLUDE+= -I$(DIRCOMMON_LOG)
OCOMMON_ALL+= $(OCOMMON_LOG)
HCOMMON_ALL+= $(HCOMMON_LOG)

##############################################################################
# Common / Sync
##############################################################################
DIRCOMMON_SYNC= $(DIRCOMMON)sync/
BCOMMON_SYNC= $(BCOMMON)sync/
_OCOMMON_SYNC= \
 atomic_mutex.o eqp_semaphore.o ipc_buffer.o share_mem.o
_HCOMMON_SYNC= server_op.h source_id.h define_share_mem.h \
 atomic_mutex.h eqp_semaphore.h ipc_buffer.h share_mem.h
OCOMMON_SYNC= $(patsubst %,$(BCOMMON_SYNC)%,$(_OCOMMON_SYNC))
HCOMMON_SYNC= $(patsubst %,$(DIRCOMMON_SYNC)%,$(_HCOMMON_SYNC))

CINCLUDE+= -I$(DIRCOMMON_SYNC)
OCOMMON_ALL+= $(OCOMMON_SYNC)
HCOMMON_ALL+= $(HCOMMON_SYNC)

##############################################################################
# Common / Time
##############################################################################
DIRCOMMON_TIME= $(DIRCOMMON)time/
BCOMMON_TIME= $(BCOMMON)time/
_OCOMMON_TIME= eqp_clock.o timer.o timer_pool.o
_HCOMMON_TIME= eqp_clock.h timer.h timer_pool.h
OCOMMON_TIME= $(patsubst %,$(BCOMMON_TIME)%,$(_OCOMMON_TIME))
HCOMMON_TIME= $(patsubst %,$(DIRCOMMON_TIME)%,$(_HCOMMON_TIME))

CINCLUDE+= -I$(DIRCOMMON_TIME)
OCOMMON_ALL+= $(OCOMMON_TIME)
HCOMMON_ALL+= $(HCOMMON_TIME)

##############################################################################
# Master
##############################################################################
DIRMASTER= src/master/
BMASTER= build/$(BUILDTYPE)/master/
_OMASTER= master_main.o \
 eqp_master.o
_HMASTER= \
 eqp_master.h
OMASTER= $(patsubst %,$(BMASTER)%,$(_OMASTER))
HMASTER= $(patsubst %,$(DIRMASTER)%,$(_HMASTER))

INCLUDEMASTER= -I$(DIRMASTER)
BINMASTER= $(DIRBIN)eqp-master

##############################################################################
# Log Writer
##############################################################################
DIRLOGWRITER= src/log/
BLOGWRITER= build/$(BUILDTYPE)/log/
_OLOGWRITER= log_main.o \
 log_writer.o log_thread.o
_HLOGWRITER= \
 log_writer.h log_thread.h
OLOGWRITER= $(patsubst %,$(BLOGWRITER)%,$(_OLOGWRITER))
HLOGWRITER= $(patsubst %,$(DIRLOGWRITER)%,$(_HLOGWRITER))

# Log Writer doesn't need much, try to minimize what we link with...
OLOGWRITER+= $(BCOMMON_TIME)eqp_clock.o $(OCOMMON_SYNC) $(OCOMMON_CONTAINER) $(BCOMMON)eqp_basic.o \
 $(BCOMMON)exception.o $(BCOMMON)eqp_thread.o $(BCOMMON)eqp_alloc.o $(BCOMMON)random.o $(BCOMMON)bit.o \
 $(BCOMMON)file.o $(BCOMMON_LOG)eqp_log.o

INCLUDELOGWRITER= -I$(DIRLOGWRITER)
BINLOGWRITER= $(DIRBIN)eqp-log-writer

##############################################################################
# Console
##############################################################################
DIRCONSOLE= src/console/
BCONSOLE= build/$(BUILDTYPE)/console/
_OCONSOLE= console_main.o \
 eqp_console.o
_HCONSOLE= \
 eqp_console.h
OCONSOLE= $(patsubst %,$(BCONSOLE)%,$(_OCONSOLE))
HCONSOLE= $(patsubst %,$(DIRCONSOLE)%,$(_HCONSOLE))

# Console doesn't need much, try to minimize what we link with...
OCONSOLE+= $(BCOMMON_TIME)eqp_clock.o $(OCOMMON_SYNC) $(BCOMMON_CONTAINER)eqp_string.o \
 $(BCOMMON)eqp_basic.o $(BCOMMON)exception.o $(BCOMMON)random.o $(BCOMMON)bit.o \
 $(BCOMMON)file.o $(BCOMMON)eqp_alloc.o  $(BCOMMON_LOG)eqp_log.o

INCLUDECONSOLE= -I$(DIRCONSOLE)
BINCONSOLE= $(DIRBIN)eqp

##############################################################################
# Core Linker flags
##############################################################################
LFLAGS= 
LSTATIC= 
LDYNAMIC= -lm -pthread -lrt -lsqlite3 -lz

##############################################################################
# Util
##############################################################################
Q= @
E= @echo
RM= rm -f 

##############################################################################
# Build rules
##############################################################################
.PHONY: default all clean

default all: master log-writer console

master: $(BINMASTER)

log-writer: $(BINLOGWRITER)

console: $(BINCONSOLE)

amalg: amalg-master amalg-log-writer amalg-console

amalg-master: $(BCOMMON)exception.o
	$(Q)luajit amalg/amalg.lua master src/master/
	$(E) "\033[0;32mCreating amalgamated source file\033[0m"
	$(E) "Building $(BINMASTER)"
	$(Q)$(CC) -o $(BINMASTER) amalg/amalg_master.c $^ $(LSTATIC) $(LDYNAMIC) $(LFLAGS) $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDEMASTER)

amalg-log-writer: $(BCOMMON)exception.o
	$(Q)luajit amalg/amalg.lua log_writer src/log/
	$(E) "\033[0;32mCreating amalgamated source file\033[0m"
	$(E) "Building $(BINLOGWRITER)"
	$(Q)$(CC) -o $(BINLOGWRITER) amalg/amalg_log_writer.c $^ $(LSTATIC) $(LDYNAMIC) $(LFLAGS) $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDELOGWRITER)

amalg-console: $(BCOMMON)exception.o
	$(Q)luajit amalg/amalg.lua console src/console/
	$(E) "\033[0;32mCreating amalgamated source file\033[0m"
	$(E) "Building $(BINCONSOLE)"
	$(Q)$(CC) -o $(BINCONSOLE) amalg/amalg_console.c $^ -lsqlite3 $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDECONSOLE)

$(BINMASTER): $(OMASTER) $(OCOMMON_ALL)
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(LSTATIC) $(LDYNAMIC) $(LFLAGS)

$(BINLOGWRITER): $(OLOGWRITER)
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(LSTATIC) $(LDYNAMIC) $(LFLAGS)

$(BINCONSOLE): $(OCONSOLE)
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(LSTATIC) $(LDYNAMIC) $(LFLAGS)

# -std=gnu11 breaks longjmp somehow...
$(BCOMMON)exception.o: $(DIRCOMMON)exception.c $(HCOMMON_ALL)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) -std=c11

$(BCOMMON)%.o: $(DIRCOMMON)%.c $(HCOMMON_ALL)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE)

$(BMASTER)%.o: $(DIRMASTER)%.c $(HMASTER)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDEMASTER)

$(BLOGWRITER)%.o: $(DIRLOGWRITER)%.c $(HLOGWRITER)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDELOGWRITER)

$(BCONSOLE)%.o: $(DIRCONSOLE)%.c $(HCONSOLE)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDECONSOLE)

##############################################################################
# Clean rules
##############################################################################
clean-common:
	$(Q)$(RM) $(BCOMMON)*.o
	$(Q)$(RM) $(BCOMMON_CONTAINER)*.o
	$(Q)$(RM) $(BCOMMON_DB)*.o
	$(Q)$(RM) $(BCOMMON_LOG)*.o
	$(Q)$(RM) $(BCOMMON_SYNC)*.o
	$(Q)$(RM) $(BCOMMON_TIME)*.o
	$(E) "Cleaned common"

clean-master:
	$(Q)$(RM) $(BMASTER)*.o
	$(Q)$(RM) $(BINMASTER)
	$(E) "Cleaned master"

clean-log-writer:
	$(Q)$(RM) $(BLOGWRITER)*.o
	$(Q)$(RM) $(BINLOGWRITER)
	$(E) "Cleaned log-writer"

clean: clean-common clean-master clean-log-writer

