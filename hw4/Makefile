all: server.out client.out

server.out: server.o Console.o
	gcc -o server.out server.o Console.o

server.o: server.c
	gcc -c -o server.o server.c

client.out: client.o Console.o
	gcc -o client.out client.o Console.o

client.o: client.c
	gcc -c -o client.o client.c

Console.o: Console.c Console.h
	gcc -c -o Console.o Console.c

clean:
	rm *.out *.o