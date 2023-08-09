all: server.out client.out

server.out: server.o Console.o trie.o
	gcc -o server.out server.o Console.o trie.o

client.out: client.o Console.o
	gcc -o client.out client.o Console.o

server.o: server.c
	gcc -c -o server.o server.c

client.o: client.c
	gcc -c -o client.o client.c

Console.o: Console.c Console.h
	gcc -c -o Console.o Console.c

trie.o: trie.c trie.h
	gcc -c -o trie.o trie.c

clean:
	rm *.out *.o