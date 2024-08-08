#
# Makefile for tcp_echo2
#
MYLIBDIR=../mynet
MYLIB=-lmynet
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: my_server

my_server: my_server.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o my_server *~