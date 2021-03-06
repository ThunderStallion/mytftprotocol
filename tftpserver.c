#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>


#define TIMEOUT 600
#define RETRYMAX 10
#define PORT 6100

char findOpcode(char packet[]);
char * getFileName(char packet[]);

int main(int argc, char **argv)
{
	char recBuffer[512];
	int rec;
	int sock;

	/*Create a socket */
	sock = socket(AF_INET,SOCK_DGRAM, 0);

	/*initialize socket structure*/
	struct sockaddr_in server;
	memset(&server, 0 , sizeof(server));
    server.sin_family = AF_INET; //AF_INET signifies Internet IP Protocol
    server.sin_addr.s_addr = htonl(INADDR_ANY); /*accepts from any address*/
    server.sin_port = htons(PORT);

	if (sock < 0){
		perror("Server: socket could not be created");
		exit(0);
	}
	/*using bind, socket now becomes a server)*/
	if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0){
		perror("bind");
		exit(EXIT_FAILURE);
	}


	for(;;){
		rec = recv(sock, recBuffer, sizeof(recBuffer),0);
		
		if(rec >0) {
			switch(getOpcode(recBuffer)){

				/*RRQ*/
				case '1':
					printf("Server: RRQ received\n");
					char * fileName = getFileName(recBuffer);
					FILE * openFile = fopen(fileName, "r");
					
					fclose(openFile);
					break;
				/*WRQ*/
				case '2':
					break;

				/*DATA*/
				case '3':
					break;
				/*ACK*/
				case '4':
					break;
				/*ERROR*/
				case '5':
					break;
				default:
					break;
			}
		}

	}
}

char getOpcode(char * packet){
	return packet[1];
}

char * getFileName(char * packet){
	 char * firstNull = strchr(packet, '\0');
	 int fileLength = firstNull - (packet+2);
	 char * fileName;
	 strncpy(fileName, packet+2, fileLength);
	 return fileName;
}

