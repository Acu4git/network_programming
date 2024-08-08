#
# Makefile for chat_server and chat_client
#
MYLIBDIR=../mynet
MYLIB=-lmynet
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}
OBJS=chat.o chat_server.o chat_client.o chat_util.o chat_common.o

all: chat

chat: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o chat *~