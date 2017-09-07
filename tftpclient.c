#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "pack_structs.h"


#define TIMEOUT 60
#define MAXPENDINGS 10
#define PORT 61005
#define MAXPACKETLENGTH 2048
#define REQUESTHDR 10

int getOpcode(char * packet);
char * getFileName(char packet[]);
short getBlockNumber(char * packet);
char * getDataPacket(char * packet, int size);
char * createConnectRequest(int type, char * filename , size_t len);
char * createAckPacket(int blockNum);
void printPacket(char * packet, int size);

int main(int argc, char *argv[])
{
	char recBuffer[MAXPACKETLENGTH];
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
		if(strcmp(argv[1], "-r")) type = 1;
		else if(strcmp(argv[2], "-w")) type = 2;
		else {
			perror("[Client] ERROR: Incorrect option for RW; use -w or -r\n");
			return 0;
		}

		/*creates first RRQ and send*/
		char * requestBuffer = createConnectRequest(1, filename, strlen(filename));
		int x = sendto(sock, requestBuffer, strlen(filename)+REQUESTHDR,
			 0, (struct sockaddr*)&serverAddress, serverAddrLen);
        printf("[Client]: %d bytes are being sent to server\n", x);
		
		
		int processComplete = 0;
		int ack_sent = 0;

		while(processComplete == 0){
			/*if succesful should receive the first data pack back*/
			printf("[Client] Waiting on Reply from Server\n");
			memset(&recBuffer, 0 , sizeof(recBuffer));
			int numOfBytesRec = recvfrom(sock, recBuffer, MAXPACKETLENGTH, 0, (struct sockaddr*)&serverAddress, &serverAddrLen);
			
			if(numOfBytesRec <= 0){
				printf("[Client]: Recvfom failed. %d returned. EXIT\n", numOfBytesRec);
				return 0;
			}
			short blockNum = getBlockNumber(recBuffer);
			printf("[CLIENT]: Packet #%d Recieved from peer -- \n %s\n", blockNum, recBuffer);
			char * ackPkt = createAckPacket(blockNum);
			printf("[Client]: Creating Ack Packet #%d", blockNum);
			ack_sent = blockNum;
			int x = sendto(sock, ackPkt, 4,
		 	0, (struct sockaddr*)&serverAddress, serverAddrLen);
    		printf("[Client]: %d bytes are being sent to server\n", x);

		}
		

	}

	return 0;
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

char * createAckPacket(int blockNum){
	char * pkt_ptr = malloc(4);
	short opCode = htons(04);
	short blockNumber = htons(blockNum);
	memcpy(pkt_ptr, &opCode , 2);
	memcpy(pkt_ptr+2, &blockNumber , 2);
	return pkt_ptr;
}

int getOpcode(char * packet){
	char  opCode = 0;
	memcpy(&opCode, packet+1, 1 * sizeof(char));
	return opCode;
}
short getBlockNumber(char * packet){
	short blockNum=0;
	memcpy(&blockNum, packet+2, 2);
	return blockNum;
}
char * getDataPacket(char * packet, int size){
	char * data = malloc(size);
	memcpy(data, packet+4, size);
	return data;
}

void printPacket(char * packet, int size){
	for(int x=0 ; x<2; x++){
		printf("[%d]: %d\n", x, packet[x]);
	}
	for(int x= 2 ; x<size; x++){
		printf("[%d]: %c\n", x, packet[x]);
	}
}

char * getFileName(char * packet){
	 char * firstNull = strchr(packet+2, '\0');
	 int fileLength = firstNull - (packet+2);
	 char * fileName = (char *) malloc(fileLength);
	 memcpy(fileName, packet+2, fileLength);
	 return fileName;
}