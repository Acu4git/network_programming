#
# Makefile for mynet
#
OBJS = init_tcpclient.o init_tcpserver.o other.o
AR = ar -qc

libmynet.a : ${OBJS}
	${RM} $@
	${AR} $@ ${OBJS}