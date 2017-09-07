CC = gcc
CFLAGS = -g -Wall 

default: all

all: server client tftplib


server : tftpserver.c pack_structs.h tftplib.h
	$(CC) $(CFLAGS) tftpserver.c -o tftpserver

tftplib : tftplib.h tftplib.c
	$(CC) $(CFLAGS) -c tftplib.c  

client : tftpclient.c pack_structs.h tftplib.h
	$(CC) $(CFLAGS) tftpclient.c -o tftpclient



clean: 
	$(RM) count *.o *~
	rm tftpserver tftpclient
