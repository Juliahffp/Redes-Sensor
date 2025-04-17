all: bin client server

bin:
	mkdir -p bin

client: client.c common.o
	gcc -Wall client.c common.o -o bin/client -lm

server: server.c common.o
	gcc -Wall server.c common.o -o bin/server -lm

common.o: common.c
	gcc -Wall -c common.c

clean:
	rm -rf bin *.o

rebuild: clean all
