PROG=example_01

CC=gcc
CFLAGS=-Wall -I../src/include -L../src
LDFLAGS=

SRC_PROG=example_01.c

CCOS=$(shell ${CC} -dumpmachine)
OBJS_PROG=$(SRC_PROG:%.c=%.o)
OBJS_LIB=$(SRC_LIB:%.c=%.o)
ifeq ($(findstring mingw32, $(CCOS)), mingw32)
LIB_SO:=$(LIB:%=%.dll)
LIB_SO_VER:=$(LIB_SO:%=%.$(VER_MAJ).$(VER_MIN))
PROG:=$(PROG).exe
endif
ifeq ($(findstring linux, $(CCOS)), linux)
LIB_SO:=$(LIB:%=lib%.so)
LIB_SO_VER:=$(LIB_SO:%=%.$(VER_MAJ).$(VER_MIN))
endif

all:	${PROG}
	echo done

${PROG}:	${OBJS_PROG}
	${CC} -static ${CFLAGS} ${LDFLAGS} -o $@ ${OBJS_PROG} -lmsdp2xxx -lm

%.o:	%.c
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm *.o ${PROG}

Makefile:
	

