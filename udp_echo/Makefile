#
# Makefile for tcp_echo2
#
MYLIBDIR=../mynet
MYLIB=-lmynet
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: echo_server1 echo_client1 echo_client2 echo_client2_sub

echo_server1: echo_server1.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

echo_client1: echo_client1.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

echo_client2: echo_client2.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

echo_client2_sub: echo_client2_sub.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o echo_client1 echo_server1 echo_client2 echo_client2_sub *~