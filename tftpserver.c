#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>


#define TIMEOUT 600
#define MAXPENDINGS 10
#define PORT 6100
#define MAXSTRINGLENGTH 512;

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
	struct sockaddr_in server;  //server address
	memset(&server, 0 , sizeof(server)); //zero out structure
    server.sin_family = AF_INET; //AF_INET signifies IPv4 address family
    server.sin_addr.s_addr = htonl(INADDR_ANY); //any incoming interface
    server.sin_port = htons(PORT); //local port

    /*setting timeout struct*/
    struct timeval tv;
    tv.tv_sec=TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));


	if (sock < 0){
		perror("Server: socket could not be created");
		exit(0);
	}
	/*using bind, socket now becomes a server)*/
	if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0){
		perror("bind");
		exit(EXIT_FAILURE);
	}
	//mark socket to listen for incoming connections
	if (listen(sock, MAXPENDING) < 0){
		perror("listen error");
		exit(EXIT_FAILURE);
	}


	for(;;){

		char buffer[MAXSTRINGLENGTH];

		struct sockaddr_in clientAddr; //client address
		socklen_t clientAddrLen = sizeof(clientAddr);

		// Size of received message
		ssize_t numBytesRcvd = recvfrom(sock, recBuffer, MAXSTRINGLENGTH, 0,
 			(struct sockaddr *) &clientAddr, &clientAddrLen);
		if (numBytesRcvd <= 0){
			perror("recvd fail");
			exit(EXIT_FAILURE);
		}

		if(numBytesRcvd >0) {
			switch(getOpcode(recBuffer)){
				/*RRQ*/
				case '1':
					printf("Server: Received [Read Request]\n");
					char * fileName = getFileName(recBuffer);
					FILE * openFile = fopen(fileName, "r");
					handleRRQ() // WRITING
					fclose(openFile);
					break;
				/*WRQ*/
				case '2':
					break;
				default:
					printf("Server: Error No RRQ/WRQ Made\n");
					break;
			}
		}

	}
	return 0;
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

char * handleRRQ( int sock, FILE * requestedFile, struct sockaddr* senderAddress){
	char outBuffer[MAXSTRINGLENGTH];
	char recBuffer[MAXSTRINGLENGTH];
	int numOfAttempts = 0;
	int blockNum = 0;

	while(dataSize > 0){
		memset(outBuffer, 0, MAXSTRINGLENGTH * sizeof(char));
		buf[0]='0';
		buf[1]='3'; //Data Packet OpCode
		buf[2-3] = blockNum;//TO DO

		int dataSize = fread(buf + 4, 1, MAXSTRINGLENGTH, requestedFile);
		if(dataSize <= 0){
			printf("RRQ: No more Data to send\n");
			break;
		}

		while(numOfAttempts < MAXATTEMPTS){

			ssize_t numBytesSent = sendto(sock, outBuffer, dataSize + 4 , 0,
				(struct sockaddr *) &clientAddr, sizeof(clientAddr));
			if (numBytesSent < 0)
			 	printf("RRQ: SendTo Failed\n");
				break;
			}
			printf("RRQ: Sending block# %d of data", blockNum);

			ssize_t numBytesRcvd = recvfrom(sock, recBuffer, MAXSTRINGLENGTH, 0,
 			(struct sockaddr *) &clientAddr, &clientAddrLen);




	}
}

