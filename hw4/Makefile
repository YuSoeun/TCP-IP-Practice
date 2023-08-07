server.out: server.o Console.o
	gcc -o server.out server.o Console.o

server.o: server.c
	gcc -c -o server.o server.c

Console.o: Console.c Console.h
	gcc -c -o Console.o Console.c

# client.out: client.o
# 	gcc -o client.out client.o

# client.o: client.c
# 	gcc -c -o client.o client.c