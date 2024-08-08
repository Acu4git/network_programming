#
# Makefile for tcp_echo2
#
MYLIBDIR=../mynet
MYLIB=-lmynet -lncurses
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: tcp_server2 tcp_client2 tcp_echo

tcp_server2: tcp_server2.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

tcp_client2: tcp_client2.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

tcp_echo: tcp_echo.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o tcp_server2 tcp_client2 *~