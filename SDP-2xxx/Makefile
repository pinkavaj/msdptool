
LIB=sdp2xxx
PROG=sdptool

CC=gcc
CFLAGS=-Wall
LDFLAGS=

SRC_PROG=sdptool.c
SRC_LIB=sdp2xxx.c

OBJS_PROG=$(SRC_PROG:%.c=%.o)
OBJS_LIB=$(SRC_LIB:%.c=%.o)
LIB_=$(LIB:%=lib%.so.0.0)

all:	${PROG}

${PROG}:	${LIB} ${OBJS_PROG}
	${CC} ${CFLAGS} ${OBJS_PROG} -l${LIB} -o $@

${LIB}:	${LIB_}

${LIB_}: ${OBJS_LIB}
	$(CC) -shared -Wl,-soname,$(LIB_) ${CFLAGS} ${LDFLAGS} -o $@ $^

%.o:	%.c
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm *.o ${LIB_} ${PROG}

