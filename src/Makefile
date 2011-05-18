VER_MAJ=0
VER_MIN=0

LIB=msdp2xxx
PROG=msdptool

CC=gcc
CFLAGS=-Wall -std=gnu99 -I./include
LDFLAGS=

SRC_PROG=msdptool.c
SRC_LIB=msdp2xxx.c msdp2xxx_low.c

OBJS_PROG=$(SRC_PROG:%.c=%.o)
OBJS_LIB=$(SRC_LIB:%.c=%.o)
LIB_SO=$(LIB:%=lib%.so)
LIB_SO_VER=$(LIB_SO:%=%.$(VER_MAJ).$(VER_MIN))

all:	${PROG}
	echo done

${PROG}:	${LIB} ${OBJS_PROG}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ -L. -lm -l${LIB} ${OBJS_PROG}

${LIB}:	${LIB_SO_VER}
	ln -sf ${LIB_SO_VER} ${LIB_SO}

${LIB_SO_VER}: ${OBJS_LIB}
	$(CC) ${CFLAGS} ${LDFLAGS} -shared -Wl,-soname,$(LIB_SO_VER) -o $@ $^

%.o:	%.c
	${CC} ${CFLAGS} -c -o $@ $<

%.c:	msdp2xxx_base.h msdp2xxx.h msdp2xxx_low.h
	

clean:
	rm *.o ${LIB_SO} ${LIB_SO_VER} ${PROG}

Makefile:
	

