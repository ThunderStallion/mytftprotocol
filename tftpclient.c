#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "tftplib.h"


char * createConnectRequest(int type, char * filename , size_t len);
int handleRRQ(int sock, char * filename, struct sockaddr_in* serverAddress, socklen_t serverAddrLen);
int handleWRQ(int sock, char * filename, struct sockaddr_in* serverAddress, socklen_t serverAddrLen);

int main(int argc, char *argv[])
{
	char * filename;
	int attempts;
	int sock;
	struct hostent *hp, *gethostbyname();
	struct sockaddr_in localAddress, serverAddress;  //Client address

	/*Create a socket */
	sock = socket(AF_INET,SOCK_DGRAM, 0);
	if (sock < 0){
		perror("[Client] socket could not be created");
		exit(0);
	}
	printf("[Client] Socket was created\n");
	/*initialize local addres structure*/
	memset(&localAddress, 0 , sizeof(localAddress)); //zero out structure
    localAddress.sin_family = AF_INET; //AF_INET signifies IPv4 address family
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddress.sin_port = htons(54332);

    /*bind socket to address*/
    if (bind(sock, (struct sockaddr *)&localAddress, sizeof(localAddress)) < 0) {
		perror("[Client] Binding socket failure. EXIT\n");
		return 0;
	}
	printf("[Client] Binding socket to address successful\n");
	/*set up serverAddress structure*/
	memset(&serverAddress, 0 , sizeof(serverAddress)); //zero out structure
    serverAddress.sin_family = AF_INET; //AF_INET signifies IPv4 address family
    hp = gethostbyname("localhost");
	bcopy ( hp->h_addr, &(serverAddress.sin_addr.s_addr), hp->h_length);
    serverAddress.sin_port = htons(PORT); //local port
    socklen_t serverAddrLen = sizeof(serverAddress);

    printf("[Client] Client has address of %u and %d \n",ntohl(localAddress.sin_addr.s_addr),
			ntohs(localAddress.sin_port));
    /*setting timeout struct*/

    struct timeval tv;
    tv.tv_sec=TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

    if(argc != 3){
    	perror("[Client] ERROR: Incorrect # of Arguments. EXIT\n");
    	return 0;
    }

	else{

		/*Everything im doing now is simply for the RRQ portion. refactor later for WRQ*/
		printf("[Client] Correct # of Arguments\n");
		filename = strdup(argv[2]);
		printf("[Client] Requesting file: %s\n", filename);

		//Choose Read or Write
		int type = -1;
		if(strcmp(argv[1], "-r") == 0)
			handleRRQ(sock, filename, &serverAddress, serverAddrLen);
		else if(strcmp(argv[1], "-w") == 0)
			handleWRQ(sock, filename, &serverAddress, serverAddrLen);
		else {
			perror("[Client] ERROR: Incorrect option for RW; use -w or -r\n");
			return 0;
		}

	}

	return 0;
}
int handleRRQ(int sock, char * filename, struct sockaddr_in* serverAddress, socklen_t serverAddrLen) {
		printf("[Client] Handling RRQ \n");
		char recBuffer[MAXPACKETLENGTH];
		short blockNum = 0;

		char * requestBuffer = createConnectRequest(1, filename, strlen(filename));
		int x = sendto(sock, requestBuffer, strlen(filename)+REQUESTHDR,
			 0, (struct sockaddr*) serverAddress, serverAddrLen);
        printf("[Client]: %d bytes are being sent to server\n", x);


        int processComplete = 0;
		int ack_sent = 0;
		int ack_rec = 0;
		while(processComplete == 0){
			/*if succesful should receive the first data pack back*/
			printf("[Client] Waiting on Reply from Server\n");
			memset(&recBuffer, 0 , sizeof(recBuffer));
			int numOfBytesRec = recvfrom(sock, recBuffer, MAXPACKETLENGTH, 0, (struct sockaddr*) &serverAddress, &serverAddrLen);

			if(numOfBytesRec <= 0){
				printf("[Client]: Recvfom failed. %d returned. EXIT\n", numOfBytesRec);
				return 0;
			}
			if(numOfBytesRec-4 <MAXDATALENGTH){
				printf("[Client] Server has signaled end of file. Closing RRQ\n");
				return 0;
			}
			short opCode = ntohs(getOpcode(recBuffer));
			printf("[Client] Received a reply from client with opcode: %d\n", opCode);

			switch(opCode){
				/*Data is received*/
				case 3:{

					short blockNum = getBlockNumber(recBuffer);
					char * message = getDataPacket(recBuffer, numOfBytesRec-4);

					printf("[CLIENT]: Packet #%d Recieved from peer -- \n %s\n", blockNum, message);

					char * ackPkt = createAckPacket(blockNum);
					printf("[Client]: Creating Ack Packet #%d\n", blockNum);
					printACKPacket(ackPkt);
					ack_sent = blockNum;
					int x = sendto(sock, ackPkt, 4, 0, (struct sockaddr*)&serverAddress, serverAddrLen);

    				printf("[Client]: %d bytes are being sent to server\n", x);
					break;

				}
				/*Ack is received*/
				case 4:{
					if(numOfBytesRec > 4){
						printf("[Client] RRQ: ACK packet size too large");
						break;
					}
					short ackNum = getBlockNumber(recBuffer);
					printf("[Client] RRQ: Received ACK %d", ackNum);
					//if ack matches block number, move on to next Data
					if(ackNum == blockNum){
						blockNum++;
						ack_rec = 1;
						break;
					}
					//
					else{
						break;
					}
					break;
				}
				/*error is received*/
				case 5:{
					char * errMsg = getErrorMessage(recBuffer);
					char * errCode = getErrorCode(recBuffer);
					printf("[Client] RRQ: [ERROR] Code %s: %s\n", errCode, errMsg);
					processComplete = 1;
					ack_rec = 1;
					break;
				}
				default:
					printf("[Client] Out of place opcode received \n");
					break;

			}

		}
		printf("[Client] Connection has end. Closing down.\n");
		return 1;
}

int handleWRQ(int sock, char * filename, struct sockaddr_in* serverAddress, socklen_t serverAddrLen) {
		printf("[Client] Handling WRQ \n");
		char recBuffer[MAXPACKETLENGTH];
		char dataToSend[MAXDATALENGTH];
		short blockNum = 0;
		int processComplete = 0;
		int ack_rec = 0;
		FILE * toSend = fopen(filename, "r");

		while (processComplete == 0) {
			while (ack_rec == 0) {
				char * requestBuffer = createConnectRequest(2, filename, strlen(filename));
				int x = sendto(sock, requestBuffer, strlen(filename)+REQUESTHDR,
				 	0, (struct sockaddr*) serverAddress, serverAddrLen);
	    	printf("[Client]: %d bytes are being sent to server\n", x);

				/*if succesful should receive an ack back*/
				printf("[Client] Waiting on Reply from Server\n");
				memset(&recBuffer, 0 , sizeof(recBuffer));
				int numOfBytesRec = recvfrom(sock, recBuffer, MAXPACKETLENGTH, 0, (struct sockaddr*) &serverAddress, &serverAddrLen);

				if(numOfBytesRec <= 0){
					printf("[Client]: Recvfom failed. %d returned. EXIT\n", numOfBytesRec);
					return 0;
				}
				short opCode = ntohs(getOpcode(&recBuffer));
				printf("[Client] Received a reply from client with opcode: %d\n", opCode);
				switch(opCode){

					/*ACK IS RECEIVED*/
					case 4:{
						if(numOfBytesRec > 4){
							printf("[Client] RRQ: ACK packet size too large");
							break;
						}
						short ackNum = getBlockNumber(recBuffer);
						printf("[Client] RRQ: Received ACK %d", ackNum);

						//if ack matches block number, move on to next Data
						if(ackNum == blockNum){
							blockNum++;
							ack_rec = 1;
							break;
						}
						else{
							break;
						}
						break;
					}
					case 5:{
						char * errMsg = getErrorMessage(recBuffer);
						char * errCode = getErrorCode(recBuffer);
						printf("[Client] RRQ: [ERROR] Code %s: %s\n", errCode, errMsg);
						processComplete = 1;
						ack_rec = 1;
						break;
					}
					default:{
						printf("[Client] Out of place opcode received \n");
						break;

					}
				}
			}

			// Now ack is received, and we can start writing data
			memset(&dataToSend, 0 , sizeof(dataToSend));
			int numBytesSent = fread(&dataToSend, sizeof(char), MAXDATALENGTH, toSend);
			if (numBytesSent < MAXDATALENGTH) {
				processComplete = 1;
			}
			int numOfAttempts = 0;
			while(numOfAttempts < MAXPENDINGS) {
				memset(&recBuffer, 0 , sizeof(recBuffer));
				char * dpkt = createDataPacket(blockNum, dataToSend, MAXDATALENGTH);

				printf("[Client] WRQ: Sending block# %d of data. Attempt #%d", blockNum, numOfAttempts);
				printf("\n[DATA]%s\n", dataToSend);
				size_t numBytesSent = sendto(sock, dpkt, MAXDATALENGTH+4 , 0,
					(struct sockaddr_in *) serverAddress, serverAddrLen);
				printf("[Client] WRQ has sent %d bytes\n", numBytesSent);
				if (numBytesSent <= 0)
			 		printf("[Client] WRQ: SendTo Failed\n");
					break;
				}

				ssize_t numBytesRcvd = recvfrom(sock, recBuffer, MAXPACKETLENGTH, 0,
 					(struct sockaddr *) &serverAddress, &serverAddrLen);

				short opCode = getOpcode(recBuffer);

				printf("[Client] Received a reply from server with opcode: %d\n", opCode);

				if(numBytesRcvd > 0){
					switch(opCode){

						/*ACK IS RECEIVED*/
						case 4:{
							if(numBytesRcvd > 4){
								printf("[Client] WRQ: ACK packet size too large");
								break;
							}
							short ackNum = getBlockNumber(recBuffer);
							printf("[Client] WRQ: Received ACK %d", ackNum);

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
							char * errCode = getErrorCode(recBuffer);
							printf("[Client] WRQ: [ERROR] Code %s: %s\n", errCode, errMsg);
							processComplete = 1;
							ack_rec = 1;
							break;
						}
						default:
							printf("[Client] WRQ: received an out of place Opcode during transmission, DISREGARD\n");
							break;

					}

				}
		}

		fclose(toSend);
		return 1;
}

char * createConnectRequest(int type, char * filename, size_t len ){
	printf("[Client] creating connection request packet\n");

	int length = len+ REQUESTHDR; //opCode[2] + filename[len] + mode[6]+zerobytes[2]
	char * pkt_ptr = malloc(length);
	short opCode = 0;
	char * mode = "octect";
	char zb1 = '\0';
	char zb2 = '\0';

	if(type == 1)
		opCode = htons(01);
	else if(type == 2)
		opCode = htons(02);

	memcpy(pkt_ptr, &opCode , 2);
	memcpy(pkt_ptr+2, filename , len);
	memcpy(pkt_ptr+2+len, &zb1 , 1);
	memcpy(pkt_ptr+3+len, mode , 6);
	memcpy(pkt_ptr+9+len, &zb2 , 1);
	printPacket(pkt_ptr, length);
	return pkt_ptr;
}
