#
# Makefile for icmp_echo
#
MYLIBDIR=../mynet
MYLIB=-lmynet
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: sping

sping: sping.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o sping *~