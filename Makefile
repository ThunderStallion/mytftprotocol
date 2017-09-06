GCC = gcc

FLAG = -o

default: all

all: server client

server:
	${GCC} tftpserver.c ${FLAG} tftpserver
client:
	${GCC} tftpclient.c ${FLAG} tftpclient
clean:
	rm tftpserver tftpclient
