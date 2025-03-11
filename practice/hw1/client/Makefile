client.out: main.o file_info.o trim.o
	gcc -o client.out main.o file_info.o trim.o
 
main.o: client.c
	gcc -c -o main.o client.c

file_info.o: file_info.h file_info.c
	gcc -c -o file_info.o file_info.c

trim.o: trim.h trim.c
	gcc -c -o trim.o trim.c