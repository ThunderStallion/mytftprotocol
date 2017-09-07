CC = gcc
CFLAGS = -g -Wall 

default: all

all: server client tftplib.o


server : tftpserver.c pack_structs.h tftplib.o tftplib.h
	$(CC) $(CFLAGS) tftpserver.c -o tftpserver tftplib.o

client : tftpclient.c pack_structs.h tftplib.o tftplib.h
	$(CC) $(CFLAGS) tftpclient.c -o tftpclient tftplib.o



clean: 
	$(RM) count *.o *~
	rm tftpserver tftpclient
