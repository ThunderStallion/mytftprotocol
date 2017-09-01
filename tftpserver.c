#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>


#define TIMEOUT 600
#define RETRYMAX 10
#define PORT 6100

main(int argc, char **argv)
{
	char buffer[512];

	/*Create a socket */
	int sock;
	sock = socket(AF_INET,SOCK_DGRAM, 0);
	int rec;

	/*initialize socket structure*/
	struct sockaddr_in server;
	memset(&server, 0 , sizeof(server));
    server.sin_family = AF_INET; //AF_INET signifies Internet IP Protocol
    server.sin_addr.s_addr = htonl(INADDR_ANY); /*accepts from anny address*/
    server.sin_port = htons(PORT);

	if (sd < 0){
		perror("Server: socket could not be created");
		exit(0);
	}
	/*using bind, socket now becomes a server)*/
	if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0){
		perror("bind");
		exit(EXIT_FAILURE);
	}


	for(;;){
		rec = recv(sock, buf, sizeof(buf),0);
		
		if(rec >0) {
			switch(findOpcode(buf)){

				/*RRQ*/
				case '1':

				/*WRQ*/
				case '2':

				/*DATA*/
				case '3':

				/*ACK*/
				case '4':

				/*ERROR*/
				case '5':

			}
		}

	}
}

char findOpcode(char packet[]){
	return packet[1];
}

