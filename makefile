CC = gcc
CFLAGS = -g -Wall 

default: all

all: server client

server : tftpserver.c pack_structs.h
	$(CC) $(CFLAGS) tftpserver.c -o tftpserver

client : tftpclient.c pack_structs.h
	$(CC) $(CFLAGS) tftpclient.c -o tftpclient

clean: 
	$(RM) count *.o *~
	rm tftpserver tftpclient
