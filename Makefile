
CC= clang

CFLAGS= 
COPT= -O2 -fomit-frame-pointer -ffast-math -std=gnu11
CWARN= -Wall -Wextra -Wredundant-decls
CWARNIGNORE= -Wno-unused-result -Wno-strict-aliasing
CINCLUDE=
CDEF=

#ifdef debug
CFLAGS+= -O0 -g -Wno-format -fno-omit-frame-pointer
CDEF+= -DEQP_DEBUG -DDEBUG
BUILDTYPE= debug
#else
#CFLAGS+= -DNDEBUG
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
 random.o zone_id.o lua_sys.o aligned.o
_HCOMMON= define.h netcode.h \
 eqp_basic.h eqp_core.h eqp_alloc.h bit.h exception.h eqp_thread.h file.h \
 random.h zone_id.h lua_sys.h aligned.h
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
_OCOMMON_CONTAINER= eqp_string.o eqp_array.o eqp_hash_table.o
_HCOMMON_CONTAINER= eqp_string.h eqp_array.h eqp_hash_table.h
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
_OCOMMON_DB= \
 database.o db_thread.o query.o
_HCOMMON_DB= transaction.h \
 database.h db_thread.h query.h
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
_OCOMMON_TIME= eqp_clock.o timer.o timer_pool.o eq_time.o
_HCOMMON_TIME= eqp_clock.h timer.h timer_pool.h eq_time.h
OCOMMON_TIME= $(patsubst %,$(BCOMMON_TIME)%,$(_OCOMMON_TIME))
HCOMMON_TIME= $(patsubst %,$(DIRCOMMON_TIME)%,$(_HCOMMON_TIME))

CINCLUDE+= -I$(DIRCOMMON_TIME)
OCOMMON_ALL+= $(OCOMMON_TIME)
HCOMMON_ALL+= $(HCOMMON_TIME)

##############################################################################
# Common / Item
##############################################################################
DIRCOMMON_ITEM= $(DIRCOMMON)item/
BCOMMON_ITEM= $(BCOMMON)item/
_OCOMMON_ITEM= item_prototype.o
_HCOMMON_ITEM= item_prototype.h
OCOMMON_ITEM= $(patsubst %,$(BCOMMON_ITEM)%,$(_OCOMMON_ITEM))
HCOMMON_ITEM= $(patsubst %,$(DIRCOMMON_ITEM)%,$(_HCOMMON_ITEM))

CINCLUDE+= -I$(DIRCOMMON_ITEM)
OCOMMON_ALL+= $(OCOMMON_ITEM)
HCOMMON_ALL+= $(HCOMMON_ITEM)

##############################################################################
# Non-Master
##############################################################################
DIRNONMASTER= src/non_master/
BNONMASTER= build/$(BUILDTYPE)/non_master/
_ONONMASTER= \
 crc.o ipc_set.o
_HNONMASTER = expansion.h tcp_packet_structs.h \
 crc.h ipc_set.h
ONONMASTER= $(patsubst %,$(BNONMASTER)%,$(_ONONMASTER))
HNONMASTER= $(patsubst %,$(DIRNONMASTER)%,$(_HNONMASTER))

INCLUDENONMASTER= -I$(DIRNONMASTER)
ONONMASTER_ALL= $(ONONMASTER)
HNONMASTER_ALL= $(HNONMASTER)

##############################################################################
# Non-Master / Net
##############################################################################
DIRNONMASTER_NET= $(DIRNONMASTER)net/
BNONMASTER_NET= $(BNONMASTER)net/
_ONONMASTER_NET= \
 packet_trilogy.o udp_socket.o udp_client.o protocol_handler.o protocol_handler_standard.o \
 protocol_handler_trilogy.o ack_mgr_standard.o ack_mgr_trilogy.o network_client.o \
 network_client_trilogy.o network_client_standard.o ack_common.o
_HNONMASTER_NET = \
 packet_trilogy.h udp_socket.h udp_client.h protocol_handler.h protocol_handler_standard.h \
 protocol_handler_trilogy.h ack_mgr_standard.h ack_mgr_trilogy.h network_client.h \
 network_client_trilogy.h network_client_standard.h ack_common.h
ONONMASTER_NET= $(patsubst %,$(BNONMASTER_NET)%,$(_ONONMASTER_NET))
HNONMASTER_NET= $(patsubst %,$(DIRNONMASTER_NET)%,$(_HNONMASTER_NET))

INCLUDENONMASTER+= -I$(DIRNONMASTER_NET)
ONONMASTER_ALL+= $(ONONMASTER_NET)
HNONMASTER_ALL+= $(HNONMASTER_NET)

##############################################################################
# Master
##############################################################################
DIRMASTER= src/master/
BMASTER= build/$(BUILDTYPE)/master/
_OMASTER= master_main.o \
 eqp_master.o master_ipc.o child_process.o console_interface.o client_manager.o client.o \
 zone_cluster.o zone_cluster_manager.o
_HMASTER= \
 eqp_master.h master_ipc.h child_process.h console_interface.h client_manager.h client.h \
 zone_cluster.h zone_cluster_manager.h
OMASTER= $(patsubst %,$(BMASTER)%,$(_OMASTER))
HMASTER= $(patsubst %,$(DIRMASTER)%,$(_HMASTER))

INCLUDEMASTER= -I$(DIRMASTER)
BINMASTER= $(DIRBIN)eqp-master

##############################################################################
# Login
##############################################################################
DIRLOGIN= src/login/
BLOGIN= build/$(BUILDTYPE)/login/
_OLOGIN= login_main.o \
 eqp_login.o login_crypto.o server_list.o login_client.o login_client_trilogy.o \
 tcp_server.o tcp_client.o client_list.o
_HLOGIN= \
 eqp_login.h login_crypto.h server_list.h login_client.h login_client_trilogy.h \
 tcp_server.h tcp_client.h client_list.h
OLOGIN= $(patsubst %,$(BLOGIN)%,$(_OLOGIN))
HLOGIN= $(patsubst %,$(DIRLOGIN)%,$(_HLOGIN))

INCLUDELOGIN= -I$(DIRLOGIN)
BINLOGIN= $(DIRBIN)eqp-login

##############################################################################
# Char Select
##############################################################################
DIRCHARSELECT= src/char_select/
BCHARSELECT= build/$(BUILDTYPE)/char_select/
_OCHARSELECT= char_select_main.o \
 eqp_char_select.o char_select_client.o char_select_client_trilogy.o tcp_client.o
_HCHARSELECT= \
 eqp_char_select.h char_select_client.h char_select_client_trilogy.h tcp_client.h
OCHARSELECT= $(patsubst %,$(BCHARSELECT)%,$(_OCHARSELECT))
HCHARSELECT= $(patsubst %,$(DIRCHARSELECT)%,$(_HCHARSELECT))

INCLUDECHARSELECT= -I$(DIRCHARSELECT)
BINCHARSELECT= $(DIRBIN)eqp-char-select

##############################################################################
# Zone Cluster
##############################################################################
DIRZC= src/zone_cluster/
BZC= build/$(BUILDTYPE)/zone_cluster/
_OZC= zone_cluster_main.o \
 zone_cluster.o zone_cluster_ipc.o zone.o lua_object.o packet_broadcast.o \
 line_of_sight_map.o
_HZC= \
 zone_cluster.h zone_cluster_ipc.h zone.h lua_object.h packet_broadcast.h \
 line_of_sight_map.h
OZC= $(patsubst %,$(BZC)%,$(_OZC))
HZC= $(patsubst %,$(DIRZC)%,$(_HZC))

INCLUDEZC= -I$(DIRZC)
BINZC= $(DIRBIN)eqp-zone-cluster

##############################################################################
# Zone Cluster / Mob
##############################################################################
DIRZC_MOB= $(DIRZC)mob/
BZC_MOB= $(BZC)mob/
_OZC_MOB= \
 mob.o client.o client_packet_standard.o inventory.o skills.o spellbook.o \
 client_packet_trilogy_input.o client_packet_trilogy_output.o npc_prototype.o \
 npc.o
_HZC_MOB= \
 mob.h client.h client_packet_standard.h inventory.h skills.h spellbook.h \
 client_packet_trilogy_input.h client_packet_trilogy_output.h npc_prototype.h \
 npc.h
OZC_MOB= $(patsubst %,$(BZC_MOB)%,$(_OZC_MOB))
HZC_MOB= $(patsubst %,$(DIRZC_MOB)%,$(_HZC_MOB))

INCLUDEZC+= -I$(DIRZC_MOB)
HZC += $(HZC_MOB)

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
 $(BCOMMON)file.o $(BCOMMON_LOG)eqp_log.o $(BCOMMON)zone_id.o

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
# Map Gen
##############################################################################
DIRMAPGEN= src/map_gen/
BMAPGEN= build/$(BUILDTYPE)/map_gen/
_OMAPGEN= map_gen_main.o \
 map_gen.o pfs.o wld.o matrix.o octree.o bsp_tree.o z_slices.o output.o
_HMAPGEN= \
 map_gen.h pfs.h wld.h matrix.h octree.h bsp_tree.h z_slices.h output.h
OMAPGEN= $(patsubst %,$(BMAPGEN)%,$(_OMAPGEN))
HMAPGEN= $(patsubst %,$(DIRMAPGEN)%,$(_HMAPGEN))

OMAPGEN+= $(BCOMMON_CONTAINER)eqp_string.o $(BCOMMON_CONTAINER)eqp_array.o \
 $(BCOMMON_CONTAINER)eqp_hash_table.o $(BCOMMON)eqp_basic.o $(BCOMMON)exception.o \
 $(BCOMMON)random.o $(BCOMMON)bit.o $(BCOMMON)file.o $(BCOMMON)eqp_alloc.o \
 $(BCOMMON_LOG)eqp_log.o $(OCOMMON_SYNC) $(BCOMMON_TIME)eqp_clock.o $(BCOMMON)aligned.o

INCLUDEMAPGEN= -I$(DIRMAPGEN)
BINMAPGEN= $(DIRBIN)eqp-map-gen

##############################################################################
# Zone Map
##############################################################################
DIRZONEMAP= src/zone_map/
BZONEMAP= build/$(BUILDTYPE)/zone_map/
_OZONEMAP= \
 aabb.o
_HZONEMAP= geometry.h map_file.h \
 aabb.h
OZONEMAP= $(patsubst %,$(BZONEMAP)%,$(_OZONEMAP))
HZONEMAP= $(patsubst %,$(DIRZONEMAP)%,$(_HZONEMAP))

OCOMMON_ALL+= $(OZONEMAP)
OMAPGEN+= $(OZONEMAP)

INCLUDEZONEMAP = -I$(DIRZONEMAP)
INCLUDEZC+= $(INCLUDEZONEMAP)
INCLUDEMAPGEN+= $(INCLUDEZONEMAP)

##############################################################################
# Core Linker flags
##############################################################################
LFLAGS= -rdynamic
LDYNAMIC= -pthread -lrt -lsqlite3 -lz -lm -ldl
LIBLUAJIT= -lluajit-5.1

ifeq ($(BUILDTYPE),debug)
LSTATIC= $(DIRBIN)libluajit-valgrind.a
LDYNCORE= $(LDYNAMIC)
else
LSTATIC= 
LDYNCORE= -lluajit-5.1 $(LDYNAMIC)
endif 

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

default all: master login char-select zone-cluster log-writer console map-gen

master: $(BINMASTER)

login: $(BINLOGIN)

char-select: $(BINCHARSELECT)

zone-cluster: $(BINZC)

log-writer: $(BINLOGWRITER)

console: $(BINCONSOLE)

map-gen: $(BINMAPGEN)

amalg: amalg-master amalg-login amalg-char-select amalg-zone-cluster amalg-log-writer amalg-console

amalg-master: $(BCOMMON)exception.o
	$(Q)luajit amalg/amalg.lua master src/master/
	$(E) "\033[0;32mCreating amalgamated source file\033[0m"
	$(E) "Building $(BINMASTER)"
	$(Q)$(CC) -o $(BINMASTER) amalg/amalg_master.c $^ $(LIBLUAJIT) $(LDYNCORE) $(LFLAGS) $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDEMASTER)

amalg-login: $(BCOMMON)exception.o
	$(Q)luajit amalg/amalg.lua login src/login/
	$(E) "\033[0;32mCreating amalgamated source file\033[0m"
	$(E) "Building $(BINLOGIN)"
	$(Q)$(CC) -o $(BINLOGIN) amalg/amalg_login.c $^ $(LSTATIC) -lcrypto $(LDYNCORE) $(LFLAGS) $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDELOGIN) $(INCLUDENONMASTER)

amalg-char-select: $(BCOMMON)exception.o
	$(Q)luajit amalg/amalg.lua char_select src/char_select/
	$(E) "\033[0;32mCreating amalgamated source file\033[0m"
	$(E) "Building $(BINCHARSELECT)"
	$(Q)$(CC) -o $(BINCHARSELECT) amalg/amalg_char_select.c $^ $(LSTATIC) $(LDYNCORE) $(LFLAGS) $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDECHARSELECT) $(INCLUDENONMASTER)

amalg-zone-cluster: $(BCOMMON)exception.o
	$(Q)luajit amalg/amalg.lua zone_cluster src/zone_cluster/
	$(E) "\033[0;32mCreating amalgamated source file\033[0m"
	$(E) "Building $(BINZC)"
	$(Q)$(CC) -o $(BINZC) amalg/amalg_zone_cluster.c $^ $(LSTATIC) $(LDYNCORE) $(LFLAGS) $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDEZC) $(INCLUDENONMASTER)

amalg-log-writer: $(BCOMMON)exception.o
	$(Q)luajit amalg/amalg.lua log_writer src/log/
	$(E) "\033[0;32mCreating amalgamated source file\033[0m"
	$(E) "Building $(BINLOGWRITER)"
	$(Q)$(CC) -o $(BINLOGWRITER) amalg/amalg_log_writer.c $^ $(LDYNAMIC) $(LFLAGS) $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDELOGWRITER)

amalg-console: $(BCOMMON)exception.o
	$(Q)luajit amalg/amalg.lua console src/console/
	$(E) "\033[0;32mCreating amalgamated source file\033[0m"
	$(E) "Building $(BINCONSOLE)"
	$(Q)$(CC) -o $(BINCONSOLE) amalg/amalg_console.c $^ $(LDYNAMIC) $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDECONSOLE)

$(BINMASTER): $(OMASTER) $(OCOMMON_ALL)
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(LIBLUAJIT) $(LDYNCORE) $(LFLAGS)

$(BINLOGIN): $(OLOGIN) $(OCOMMON_ALL) $(ONONMASTER_ALL)
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(LSTATIC) -lcrypto $(LDYNCORE) $(LFLAGS)

$(BINCHARSELECT): $(OCHARSELECT) $(OCOMMON_ALL) $(ONONMASTER_ALL)
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(LSTATIC) $(LDYNCORE) $(LFLAGS)

$(BINZC): $(OZC) $(OZC_MOB) $(OCOMMON_ALL) $(ONONMASTER_ALL)
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(LSTATIC) $(LDYNCORE) $(LFLAGS)

$(BINLOGWRITER): $(OLOGWRITER)
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(LSTATIC) $(LDYNAMIC) $(LFLAGS)

$(BINCONSOLE): $(OCONSOLE)
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(LSTATIC) $(LDYNAMIC) $(LFLAGS)

$(BINMAPGEN): $(OMAPGEN)
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(LDYNAMIC)

# -std=gnu11 breaks longjmp somehow...
$(BCOMMON)exception.o: $(DIRCOMMON)exception.c $(HCOMMON_ALL)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) -std=c11

$(BCOMMON)%.o: $(DIRCOMMON)%.c $(HCOMMON_ALL)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE)

$(BNONMASTER)%.o: $(DIRNONMASTER)%.c $(HNONMASTER_ALL)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDENONMASTER)

$(BMASTER)%.o: $(DIRMASTER)%.c $(HMASTER)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDEMASTER)

$(BLOGIN)%.o: $(DIRLOGIN)%.c $(HLOGIN)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDELOGIN) $(INCLUDENONMASTER)

$(BCHARSELECT)%.o: $(DIRCHARSELECT)%.c $(HCHARSELECT)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDECHARSELECT) $(INCLUDENONMASTER)

$(BZC)%.o: $(DIRZC)%.c $(HZC)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDEZC) $(INCLUDENONMASTER)

$(BLOGWRITER)%.o: $(DIRLOGWRITER)%.c $(HLOGWRITER)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDELOGWRITER)

$(BCONSOLE)%.o: $(DIRCONSOLE)%.c $(HCONSOLE)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDECONSOLE)

$(BMAPGEN)%.o: $(DIRMAPGEN)%.c $(HMAPGEN)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDEMAPGEN)

$(BZONEMAP)%.o: $(DIRZONEMAP)%.c $(HZONEMAP)
	$(E) "\033[0;32mCC      $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDEZONEMAP)

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

clean-non-master:
	$(Q)$(RM) $(BNONMASTER)*.o
	$(Q)$(RM) $(BNONMASTER_NET)*.o
	$(E) "Cleaned non-master"

clean-master:
	$(Q)$(RM) $(BMASTER)*.o
	$(Q)$(RM) $(BINMASTER)
	$(E) "Cleaned master"

clean-login:
	$(Q)$(RM) $(BLOGIN)*.o
	$(Q)$(RM) $(BINLOGIN)
	$(E) "Cleaned login"

clean-char-select:
	$(Q)$(RM) $(BCHARSELECT)*.o
	$(Q)$(RM) $(BINCHARSELECT)
	$(E) "Cleaned char-select"

clean-zone-cluster:
	$(Q)$(RM) $(BZC)*.o
	$(Q)$(RM) $(BZC_MOB)*.o
	$(Q)$(RM) $(BZONEMAP)*.o
	$(Q)$(RM) $(BINZC)
	$(E) "Cleaned zone-cluster"

clean-log-writer:
	$(Q)$(RM) $(BLOGWRITER)*.o
	$(Q)$(RM) $(BINLOGWRITER)
	$(E) "Cleaned log-writer"

clean-console:
	$(Q)$(RM) $(BCONSOLE)*.o
	$(Q)$(RM) $(BINCONSOLE)
	$(E) "Cleaned console"

clean-map-gen:
	$(Q)$(RM) $(BMAPGEN)*.o
	$(Q)$(RM) $(BZONEMAP)*.o
	$(Q)$(RM) $(BINMAPGEN)
	$(E) "Cleaned map-gen"

clean: clean-common clean-non-master clean-master clean-login clean-char-select clean-zone-cluster clean-log-writer clean-console clean-map-gen

