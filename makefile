CC = gcc
CFLAGS = -g -Wall 

server : tftpserver.c pack_structs.h
	$(CC) $(CFLAGS) -o tftpserver tftpserver.c

client : tftpclient.c pack_structs.h
	$(CC) $(CFLAGS) -o tftpclient tftpclient.c

clean: 
	$(RM) count *.o *~
