server.out: main.o
	gcc -o server.out main.o
 
main.o: packet.h server.c
	gcc -c -o main.o server.c