
LIB=msdp2xxx
PROG=msdptool

CC=gcc
CFLAGS=-Wall -std=gnu99
LDFLAGS=

SRC_PROG=msdptool.c
SRC_LIB=msdp2xxx.c msdp2xxx_low.c

OBJS_PROG=$(SRC_PROG:%.c=%.o)
OBJS_LIB=$(SRC_LIB:%.c=%.o)
LIB_=$(LIB:%=lib%.so.0.0)

all:	${PROG}
	

${PROG}:	${LIB} ${OBJS_PROG}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ -L. -lm -l${LIB} ${OBJS_PROG}

${LIB}:	${LIB_}
	ln -sf ${LIB_} ${LIB}

${LIB_}: ${OBJS_LIB}
	$(CC) ${CFLAGS} ${LDFLAGS} -shared -Wl,-soname,$(LIB_) -o $@ $^

%.o:	%.c
	${CC} ${CFLAGS} -c -o $@ $<

%.c:	msdp2xxx_base.h  msdp2xxx.h  msdp2xxx_low.h
	

%.h:
	

clean:
	rm *.o ${LIB} ${LIB_} ${PROG}

