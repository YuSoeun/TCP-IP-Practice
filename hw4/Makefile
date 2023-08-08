all: server.out client.out

server.out: server.o Console.o file.o trie.o
	gcc -o server.out server.o Console.o file.o trie.o

server.o: server.c
	gcc -c -o server.o server.c

client.out: client.o Console.o
	gcc -o client.out client.o Console.o

client.o: client.c
	gcc -c -o client.o client.c

Console.o: Console.c Console.h
	gcc -c -o Console.o Console.c

file.o: file.c file.h
	gcc -c -o file.o file.c

trie.o: trie.c trie.h
	gcc -c -o trie.o trie.c

clean:
	rm *.out *.o