OBJS=main.o mynet.o client.o

all: main

main: ${OBJS}
	${CC}  -o $@ $^

clean:
	${RM} *.o main *~