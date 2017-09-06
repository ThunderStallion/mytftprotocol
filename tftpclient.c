#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "pack_structs.h"


#define TIMEOUT 600
#define MAXPENDINGS 10
#define PORT 61008
#define MAXSTRINGLENGTH 512;

char getOpcode(char * packet);
char * getFileName(char packet[]);
char * createConnectRequest(int type, char * filename , size_t len);
void printPacket(char * packet);

int main(int argc, char *argv[])
{
	char recBuffer[512];
	char * filename;
	int sock;
	struct hostent *hp, *gethostbyname();
	struct sockaddr_in localAddress, serverAddress;  //Client address

	/*Create a socket */
	sock = socket(AF_INET,SOCK_DGRAM, 0);
	if (sock < 0){
		perror("Server: socket could not be created");
		exit(0);
	}
	printf("[CHECK] Socket was created\n");
	/*initialize local addres structure*/
	memset(&localAddress, 0 , sizeof(localAddress)); //zero out structure
    localAddress.sin_family = AF_INET; //AF_INET signifies IPv4 address family
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddress.sin_port = htons(55532);

    /*bind socket to address*/
    if (bind(sock, (struct sockaddr *)&localAddress, sizeof(localAddress)) < 0) {
		perror("bind failed");
		return 0;
	}
	printf("[CHECK] Bind successful\n");
	/*set up serverAddress structure*/
	memset(&serverAddress, 0 , sizeof(serverAddress)); //zero out structure
    serverAddress.sin_family = AF_INET; //AF_INET signifies IPv4 address family
    hp = gethostbyname("localhost");
	bcopy ( hp->h_addr, &(serverAddress.sin_addr.s_addr), hp->h_length);
    serverAddress.sin_port = htons(PORT); //local port
    socklen_t serverAddrLen = sizeof(serverAddress);

    /*setting timeout struct*/
    struct timeval tv;
    tv.tv_sec=TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

    if(argc != 3){
    	perror("[ERROR] Incorrect # of Arguments\n");
    	return 0;
    }

	else{
		printf("[CHECK] Correct # of Arguments\n");
		filename = strdup(argv[2]);
		printf("[CHECK] The filename is %s\n", filename);

		int type = -1;
		if(strcmp(argv[1], "-r")) type = 1;
		else if(strcmp(argv[2], "-w")) type = 2;
		else {
			perror("[ERROR] Incorrect option for RW; use -w or -r\n");
			return 0;
		}
		char * requestBuffer = createConnectRequest(type, filename, strlen(filename));
		int x = sendto(sock, requestBuffer, strlen(requestBuffer), 0, (struct sockaddr*)&serverAddress, serverAddrLen);
    printf("Client: Request sent to server\n");

		int numOfBytesRec = recvfrom(sock, recBuffer, 2048, 0, (struct sockaddr*)&serverAddress, &serverAddrLen);
		if(numOfBytesRec <= 0){
			printf("Client: RECVFROM fail\n");

		}
		else{
			printf("CLIENT: %s\n", recBuffer);
		}

	}

	return 0;
}

char * createConnectRequest(int type, char * filename, size_t len ){
	printf("[CHECK] In connect Request\n");

	int length = len +2+ 6+2;
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
	printPacket(pkt_ptr);
	return pkt_ptr;
}

char getOpcode(char * packet){
	char  opCode = 0;
	memcpy(&opCode, packet+1, 1 * sizeof(char));
	return opCode;
}

void printPacket(char * packet){
	for(int x=0 ; x<2; x++){
		printf("[%d]: %d\n", x, packet[x]);
	}
	for(int x= 2 ; x<17; x++){
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
