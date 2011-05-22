VER_MAJ=0
VER_MIN=0

LIB=msdp2xxx
PROG=msdptool

AR=ar
CC=gcc
#CC=i486-mingw32-gcc
INSTALL=install
CFLAGS=-Wall -std=gnu99 -I./include
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
LIB_SO:=$(LIB:%=%.dll)
LIB_SO_VER:=$(LIB_SO:%=%.$(VER_MAJ).$(VER_MIN))
LIB_STATIC=${LIB_SO:%.dll=%.lib}
PROG:=$(PROG).exe
endif
ifeq ($(findstring linux, $(CCOS)), linux)
LIB_SO:=$(LIB:%=lib%.so)
LIB_SO_VER:=$(LIB_SO:%=%.$(VER_MAJ).$(VER_MIN))
LIB_STATIC=${LIB_SO:%.so=%.a}
endif

all:	${PROG}
	echo done

${PROG}:	${LIB} ${OBJS_PROG}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ -L. -lm -l${LIB} ${OBJS_PROG}

${LIB}:	${LIB_SO_VER}
	ln -sf ${LIB_SO_VER} ${LIB_SO}

${LIB_SO_VER}: ${OBJS_LIB}
	$(CC) ${CFLAGS} ${LDFLAGS} -shared -Wl,-soname,$(LIB_SO_VER) -o $@ $^

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
	$(INSTALL) -m +x $(LIB_SO_VER) $(LIB_DIR)

