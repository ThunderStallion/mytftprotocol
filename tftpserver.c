#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pack_structs.h"
#include <netinet/in.h>
#include <sys/socket.h>


#define TIMEOUT 600
#define MAXPENDINGS 10
#define PORT 6100
#define MAXDATALENGTH 512
#define MAXPACKETLENGTH 2024

char getOpcode(char packet[]);
short getBlockNum(char * packet);
char * getFileName(char packet[]);
char * getErrorMessage(char * packet);
void handleRRQ( int sock, FILE * requestedFile, struct sockaddr_in* clientAddr, socklen_t clientAddrLen);


int main(int argc, char **argv)
{
	char recBuffer[MAXPACKETLENGTH];
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
	
	printf("Server will now start looping\n");
	for(;;){

		char buffer[MAXDATALENGTH];

		struct sockaddr_in clientAddr; //client address
		socklen_t clientAddrLen = sizeof(clientAddr);

		// Size of received message
		ssize_t numBytesRcvd = recvfrom(sock, recBuffer, MAXPACKETLENGTH, 0,
 			(struct sockaddr *) &clientAddr, &clientAddrLen);
		if (numBytesRcvd <= 0){
			perror("recvd fail");
			exit(EXIT_FAILURE);
		}

		if(numBytesRcvd >0) {
			switch(getOpcode(recBuffer)){
				/*RRQ*/
				case 1:{
					printf("Server: Received [Read Request]\n");

					FILE * openFile = fopen(getFileName(recBuffer), "r");
					handleRRQ(sock, openFile,&clientAddr,  clientAddrLen ); // WRITING
					fclose(openFile);

					printf("Server: Transmission Complete [Read Request]\n");
					break;
				}
				/*WRQ*/
				case 2:{
					break;
				}
				default:{
					printf("Server: Error No RRQ/WRQ Made\n");
					break;
				}
			}
		}

	}
	return 0;
}

char  getOpcode(char * packet){
	char  opCode = 0;
	memcpy(&opCode, packet+1, 1 * sizeof(char));

	return opCode;
}

short getBlockNum(char * packet){
	short blockNum = 0;
	memcpy(&blockNum, packet+2, 1 * sizeof(short));
	return blockNum;

}
char * getFileName(char * packet){
	 char * firstNull = strchr(packet+2, '\0');
	 int fileLength = firstNull - (packet+2);
	 char * fileName = (char *) malloc(fileLength);
	 memcpy(fileName, packet+2, fileLength);
	 return fileName;
}

char * getErrorMessage(char * packet){
	char * firstNull = strchr(packet+4,'\0');
	int errMsgLen = firstNull - (packet+4);
	printf("%d\n", errMsgLen);
	char * errMsg = (char *) malloc(errMsgLen);
	memcpy(errMsg, packet+4, errMsgLen);
	return errMsg;
}

void handleRRQ( int sock, FILE * requestedFile, struct sockaddr_in* clientAddr, socklen_t clientAddrLen){
	char outBuffer[MAXDATALENGTH];
	char recBuffer[MAXDATALENGTH];
	short blockNum = 1;
	int sendComplete = 0;

	while(sendComplete==0){

		memset(outBuffer, 0, MAXDATALENGTH * sizeof(char));
		int dataSize = fread(outBuffer, 1, MAXDATALENGTH, requestedFile);
		if(dataSize <= 0){
			printf("RRQ: No more Data to send\n");
			sendComplete =1;
			break;
		}

		int numOfAttempts = 0;
		int ack_rec = 0;

		while(numOfAttempts < MAXPENDINGS && ack_rec == 0){
			struct DATAPacket pkt_struct;
			pkt_struct.opCode = htons(3);
			pkt_struct.block_num = htons(blockNum);
			memcpy(pkt_struct.data, outBuffer, dataSize);
			char * dpkt = (char *)(&pkt_struct);

			ssize_t numBytesSent = sendto(sock, dpkt, dataSize + 4 , 0,
				(struct sockaddr *) &clientAddr, clientAddrLen);

			if (numBytesSent < 0)
			 	printf("RRQ: SendTo Failed\n");
				break;
			}
			printf("RRQ: Sending block# %d of data. Attempt #%d", blockNum, numOfAttempts);

			ssize_t numBytesRcvd = recvfrom(sock, recBuffer, MAXPACKETLENGTH, 0,
 			(struct sockaddr *) &clientAddr, &clientAddrLen);
			


			if(numBytesRcvd > 0){
				switch(getOpcode(recBuffer)){

					/*ACK IS RECEIVED*/
					case 4:{ 
						if(numBytesRcvd > 4){
							printf("RRQ: ACK packet size too large");
							break;
						}
						short ackNum = getBlockNum(recBuffer);
						printf("RRQ: Received ACK %d", ackNum);
						//if ack matches block number, move on to next Data
						if(ackNum == blockNum){
							blockNum++;
							ack_rec = 1;
							break;
						}
						//
						else{
							numOfAttempts++;
							break;
						}	
						break;
					}
					case 5:{
						char * errMsg = getErrorMessage(recBuffer);
						printf("RRQ: [ERROR] %s\n", errMsg);
						sendComplete = 1;
						ack_rec = 1;
						break;
					}
					default:
						printf("RRQ: received an out of place Opcode during transmission, DISREGARD\n");
						break;
						
				}
			}

			if(numOfAttempts == MAXPENDINGS){
				printf("RRQ: Exceeded Max attempts");

			}
			/*TO DO: If MAX-ATTEMPTS = 10 , send error Message*/
			/*TO DO: Timer*/
	}
	return;
}

