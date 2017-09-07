#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "tftplib.h"
#include <errno.h>



void handleRRQ( int sock, char * filename, struct sockaddr_in clientAddr, socklen_t clientAddrLen);
void handleWRQ(int sock, FILE * requestedFile, struct sockaddr_in* clientAddr, socklen_t clientAddrLen);

int main(int argc, char **argv)
{
	char recBuffer[MAXPACKETLENGTH];
	int sock;

	/*Create a socket */
	sock = socket(AF_INET,SOCK_DGRAM, 0);
	if (sock < 0){
		perror("[Server] socket could not be created");
		exit(0);
	}
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

	/*using bind, socket now becomes a server)*/
	if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0){
		perror("[Server] Binding socket failure. EXIT\n");
		return 0;
	}
	//mark socket to listen for incoming connections

	printf("[Server] Connection Successful, awaiting Requests\n");
	for(;;){

		struct sockaddr_in clientAddr; //client address
		socklen_t clientAddrLen = sizeof(clientAddr);

		// Size of received message
		ssize_t numBytesRcvd = recvfrom(sock, recBuffer, MAXPACKETLENGTH, 0,
 			(struct sockaddr *) &clientAddr, &clientAddrLen);

		if (numBytesRcvd < 0){
			perror("[Server] recvd fail");
			exit(EXIT_FAILURE);
		}

		short opCode = ntohs(getOpcode(recBuffer));
		printf("[Server] Received a reply from client with opcode: %d\n", opCode);
		printPacket(recBuffer, numBytesRcvd);
		printf("[Server] Client has port of %d  \n", ntohs(clientAddr.sin_port));

		if(numBytesRcvd >0) {
			switch(opCode){
				/*RRQ*/
				case 1:{
					printf("[Server] Received [Read Request]\n");

					
					handleRRQ(sock, getFileName(recBuffer), clientAddr,  clientAddrLen ); // WRITING

					printf("[Server] Transmission Complete.\n");
					break;
				}
				/*WRQ*/
				case 2:{
					printf("[Server] Received [Write Request]\n");

					FILE * openFile = fopen(getFileName(recBuffer), "w");
					handleWRQ(sock, openFile, &clientAddr,  clientAddrLen ); // WRITING
					fclose(openFile);

					printf("[Server] Transmission Complete [Write Request]\n");
					break;
				}
				default:{
					printf("[Server] Error. Please make a Read/Write request first\n");
					return 0;
				}
			}
		}

	}
	return 0;
}



void handleRRQ( int sock, char * filename, struct sockaddr_in clientAddr, socklen_t clientAddrLen){
	char fileBuffer[MAXDATALENGTH]; //buffer to read in file
	char recBuffer[MAXPACKETLENGTH]; //buffer to receive messages from client
	short blockNum = 0;
	int sendComplete = 0; //bool to continue loop
	FILE * requestedFile = fopen(filename, "r");
	time_t start;

	while(sendComplete==0){

		memset(&fileBuffer, 0, MAXDATALENGTH * sizeof(char)); //reset buffer to 0
		int dataSize = fread(&fileBuffer, sizeof(char), MAXDATALENGTH, requestedFile);

		if(dataSize <= 0){
			printf("[Server] RRQ: No more Data to send\n");
			sendComplete=1;
			return;
		}

		int numOfAttempts = 0;
		int ack_rec = 0;
		blockNum++;


		while(numOfAttempts <= MAXPENDINGS && ack_rec == 0){
			
			if(numOfAttempts == MAXPENDINGS){
				printf("[Server] RRQ: Exceeded Max attempts. Exiting\n");
				return;
			}
			start = clock();
			memset(&recBuffer, 0, sizeof(recBuffer));
			char * dpkt = createDataPacket(blockNum, fileBuffer, MAXDATALENGTH);
			printf("[Server] RRQ: Sending block#%d of data. Attempt #%d\n", blockNum, numOfAttempts);
			size_t numBytesSent = sendto(sock, dpkt, dataSize+4, 0,
				(struct sockaddr*) &clientAddr, clientAddrLen);
			printf("[Server] RRQ has sent %zu bytes\n", numBytesSent);
			if (numBytesSent < 0){
			 	printf("[Server] RRQ: SendTo Failed\n [ERROR] %s\n", strerror(errno));
				return;
			}
			
			if(clock() - start > TIMEOUT){
				numOfAttempts++;
				continue;
			}

			size_t numBytesRcvd = recvfrom(sock, recBuffer, MAXPACKETLENGTH, 0,
 			(struct sockaddr *) &clientAddr, &clientAddrLen);

			short opCode = ntohs(getOpcode(recBuffer));
			short blockNum = ntohs(getBlockNumber(recBuffer));

			printf("[Server] Received a reply from client for blockNum: %d \n", opCode, blockNum);
			if(numBytesRcvd < 0){
				printf("[Server] Error receiving from client, reattempt.");
				numOfAttempts++;
				break;
			}
		
			switch(opCode){
				/*ACK IS RECEIVED*/
				case 4:{
					if(numBytesRcvd > 4){
						printf("[Server] RRQ: ACK packet size too large");
						break;
					}					
					short ackNum = ntohs(getBlockNumber(recBuffer));
					printf("[Server] RRQ: Received ACK %d\n", ackNum);
					//if ack matches block number, move on to next Data
					if(ackNum == blockNum){
						ack_rec = 1;
					}
					//else retry again
					else{
						numOfAttempts++;
					}
					break;
				}
				case 5:{
					char * errMsg = getErrorMessage(recBuffer);
					printf("[Server] RRQ: [ERROR] %s\n", errMsg);
					sendComplete = 1;
					ack_rec = 1;
					return;
					break;
				}
				default:{
					printf("[Server] RRQ: received an out of place Opcode during transmission, DISREGARD\n");
					numOfAttempts++;
					break;
				}
			}	
		}
	}
	fclose(requestedFile);
	printf("[Server] RRQ: Finished. \n");
	return;
}

// Receiving file from client
void handleWRQ( int sock, FILE * requestedFile, struct sockaddr_in* clientAddr, socklen_t clientAddrLen){

	char dataBuffer[MAXDATALENGTH];
	char inBuffer[MAXDATALENGTH + 4];
	int blockNum = 1;
	int receiptComplete = 0;

	while(receiptComplete == 0){

		int numAttempts = 0;
		while(numAttempts < MAXPENDINGS) {
			memset(&inBuffer, 0, MAXDATALENGTH * sizeof(char));
			printf("WRQ: Sending block # %d of ACK", blockNum);
			char * ackpkt = createAckPacket(blockNum);
			ssize_t numBytesSent = sendto(sock, ackpkt, ACKSIZE, 0,
				(struct sockaddr *) &clientAddr, clientAddrLen);
			if (numBytesSent != ACKSIZE) {
				printf("WRQ: SendTo Failed\n");
				break;
			}

			ssize_t numBytesRcvd = recvfrom(sock, inBuffer, MAXDATALENGTH + 4, 0,
 			NULL, NULL);
			if(numBytesRcvd >= 0){

				memset(&dataBuffer, 0, MAXDATALENGTH * sizeof(char));
				char * data = getDataPacket(inBuffer, numBytesRcvd - 4);
				short recBlockNum = getBlockNumber(inBuffer);
				printf("WRQ: Received Block %d", recBlockNum);
				memcpy(&dataBuffer, data, numBytesRcvd - 4);
				if ((numBytesRcvd - 4) < MAXDATALENGTH) {
					printf("WRQ: No more Data to receive\n");
					receiptComplete = 1;
				}
				if (recBlockNum > blockNum) {
					blockNum = recBlockNum;
					fwrite(data, sizeof(char), numBytesRcvd - 4, requestedFile);
					break;
				}

			}

		}
	}

	printf("WRQ: Sending block # %d of ACK\n", blockNum);
	char * ackpkt = createAckPacket(blockNum);
	ssize_t numBytesSent = sendto(sock, ackpkt, ACKSIZE, 0,
		(struct sockaddr *) &clientAddr, clientAddrLen);
	printf(numBytesSent);
	if (numBytesSent != ACKSIZE) {
		printf("WRQ: SendTo Failed\n");
	}

	return;
}