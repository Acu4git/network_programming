#
# Makefile for tcp_echo2
#
MYLIBDIR=../mynet
MYLIB=-lmynet
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: my_server_with_fork my_server_with_thread

my_server_with_fork: my_server_with_fork.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

my_server_with_thread: my_server_with_thread.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o my_server_with_fork my_server_with_thread *~