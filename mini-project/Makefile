# all: server.out client.out

p2p: server.o client.o Console.o file.o main.c
	gcc -o p2p server.o client.o Console.o main.c file.o

server.o: server.c
	gcc -c -o server.o server.c

client.o: client.c
	gcc -c -o client.o client.c

Console.o: Console.c Console.h
	gcc -c -o Console.o Console.c

file.o: file.c file.h
	gcc -c -o file.o file.c

clean:
	rm *.out *.o