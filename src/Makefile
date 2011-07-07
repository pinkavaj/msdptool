VER_MAJ=0
VER_MIN=0

LIB=msdp2xxx
PROG=msdptool

#CROSS_COMPILE=i486-mingw32-

AR=$(CROSS_COMPILE)ar
CC=$(CROSS_COMPILE)gcc
INSTALL=install
CFLAGS=-Wall -std=gnu99 -I./include -fPIC
LDFLAGS=

SRC_PROG=msdptool.c
SRC_LIB=msdp2xxx.c msdp2xxx_low.c

prefix=/usr/local
BIN_DIR=$(prefix)/bin
LIB_DIR=$(prefix)/lib

CCOS=$(shell ${CC} -dumpmachine)
OBJS_PROG=$(SRC_PROG:%.c=%.o)
OBJS_LIB=$(SRC_LIB:%.c=%.o)
ifeq ($(findstring mingw32, $(CCOS)), mingw32)
LIB_DINAMIC:=$(LIB:%=%.dll)
LIB_NAME:=${LIB_DINAMIC}
LIB_STATIC=${LIB_DINAMIC:%.dll=lib%.a}
PROG:=$(PROG).exe
endif
ifeq ($(findstring linux, $(CCOS)), linux)
LIB_LN:=$(LIB:%=lib%.so)
LIB_NAME:=${LIB_LN}
LIB_DINAMIC:=$(LIB_LN).$(VER_MAJ).$(VER_MIN)
LIB_STATIC=${LIB_LN:%.so=%.a}
endif

all:	${PROG}
	echo done

${PROG}:	${LIB} ${OBJS_PROG}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ -L. ${OBJS_PROG} -lm -l${LIB}

${LIB}: $(LIB_DINAMIC)	$(LIB_STATIC)
	[ "${LIB_LN}_" == "_" ] || ln -sf ${LIB_DINAMIC} $(LIB_LN)

${LIB_DINAMIC}: ${OBJS_LIB}
	$(CC) ${CFLAGS} ${LDFLAGS} -shared -Wl,-soname,$(LIB_NAME) -o $@ $^

${LIB_STATIC}: ${OBJS_LIB}
	$(AR) rcs $(LIB_STATIC) $^

%.o:	%.c
	${CC} ${CFLAGS} -c -o $@ $<

%.c:	msdp2xxx_base.h msdp2xxx.h msdp2xxx_low.h
	

clean:
	for f in "*.a" \
		"*.dll" \
		"*.exe" \
		"*.lib" \
		"*.o" \
		"*.o" \
		"*.so" \
		"*.so.*" \
		${PROG}; \
		do find . -name "$${f}" -exec rm \{\} \; ; done

Makefile:
	

install: all
	$(INSTALL) -m +x $(PROG) $(BIN_DIR)
	$(INSTALL) -m +x $(LIB_STATIC) $(LIB_DIR)
	$(INSTALL) -m +x $(LIB_DINAMIC) $(LIB_DIR)

