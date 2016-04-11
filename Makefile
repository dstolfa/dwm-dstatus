TARGET = dwm-dstatus 
SRC = ${wildcard *.c}
OBJ = ${SRC:.c=.o}
HDR = ${wildcard *.h}
ERR = $(shell which gcc >/dev/null; echo $$?)
ifeq "$(ERR)" "0"
     CC = gcc 
else
     CC = clang 
endif

REVCNT = $(shell git rev-list --count master 2>/dev/null)
REVHASH = $(shell git log -1 --format="%h" 2>/dev/null)
ifeq (${REVCNT},)
	VERSION = devel
else
	VERSION = "${REVCNT}.${REVHASH}"
endif

CFLAGS = -Wall
LFLAGS = -lX11
INSTALL = install
INSTALL_ARGS = -o root -g root -m 755
INSTALL_DIR = /usr/local/bin/

ifeq (${CC}, $(filter ${CC}, cc gcc clang))
CFLAGS += -std=c99 -pedantic
endif

# autoconfiguration
BATPATH=`find /sys/class/power_supply/ -name BAT1 -print0 -quit`
WLLNKPATH=`find /sys/class/net/wlp2s0/ -name operstate -print0 -quit`
ETHLNKPATH=`find /sys/class/net/enp1s0/ -name operstate -print0 -quit`

all: debug

debug: CFLAGS += -g -DDEBUG
debug: LFLAGS += -g
debug: build

release: CFLAGS += -O3
release: LFLAGS += -lX11
release: clean build

build: helpers_defs.h ${TARGET}

helpers_defs.h:
	@echo "#define BUILD_HOST \"`hostname`\""                                 > helpers_defs.h
	@echo "#define BUILD_OS \"`uname`\""                                     >> helpers_defs.h
	@echo "#define BUILD_PLATFORM \"`uname -m`\""                            >> helpers_defs.h
	@echo "#define BUILD_KERNEL \"`uname -r`\""                              >> helpers_defs.h
	@echo "#define BUILD_VERSION \"${VERSION}\""                             >> helpers_defs.h
	@echo "#define BAT_NOW \"${BATPATH}/capacity\""                          >> helpers_defs.h
	@echo "#define BAT_ENERGY_FULL \"${BATPATH}/energy_full\""               >> helpers_defs.h
	@echo "#define BAT_ENERGY_FULL_DESIGN \"${BATPATH}/energy_full_design\"" >> helpers_defs.h
	@echo "#define BAT_STAT \"${BATPATH}/status\""                           >> helpers_defs.h
	@echo "#define WL_LNK_PATH \"${WLLNKPATH}\""                             >> helpers_defs.h
	@echo "#define ETH_LNK_PATH \"${ETHLNKPATH}\""                           >> helpers_defs.h

install: release
	${INSTALL} ${INSTALL_ARGS} ${TARGET} ${INSTALL_DIR}
	@echo "DONE"

${TARGET}: helpers_defs.h ${OBJ}
	${CC} -o $@ ${OBJ} ${LFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c $?

clean:
	-rm -f helpers_defs.h
	-rm -f *.o ${TARGET}

.PHONY: all debug release build install clean

