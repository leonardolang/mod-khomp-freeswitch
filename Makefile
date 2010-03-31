MODNAME=mod_khomp
VERBOSE=1
LOCAL_CFLAGS=-I./include -I./commons -D_REENTRANT -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DK3L_HOSTSYSTEM -DCOMMONS_LIBRARY_USING_FREESWITCH -g -ggdb
LOCAL_LDFLAGS=-lk3l
LOCAL_OBJS= ./commons/k3lapi.o ./commons/k3lutil.o ./commons/config_options.o ./commons/format.o ./commons/strings.o ./commons/ringbuffer.o ./commons/verbose.o ./commons/saved_condition.o ./commons/regex.o
LOCAL_OBJS+= ./src/globals.o ./src/opt.o ./src/frame.o ./src/utils.o ./src/lock.o ./src/spec.o ./src/khomp_pvt_kxe1.o ./src/khomp_pvt.o ./src/logger.o

ifeq ($(strip $(FREESWITCH_PATH)),)
	BASE=../../../../
else
	BASE=$(FREESWITCH_PATH)
endif

include $(BASE)/build/modmake.rules
