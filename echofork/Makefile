#
# Makefile for tcp_echo2
#
MYLIBDIR=../mynet
MYLIB=-lmynet
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: echofork

echofork: echofork.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o echofork *~