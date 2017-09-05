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

char findOpcode(char packet[]);
char * getFileName(char packet[]);
char * createConnectRequest(int type, char * filename );

int main(int argc, char *argv[])
{
	char recBuffer[512];
	char * filename;
	int rec;
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

		char * requestBuffer = createConnectRequest(1, filename);
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

char * createConnectRequest(int type, char * filename ){
	printf("[CHECK] In connect Request\n");


	struct RQPacket * pkt_struct;
	pkt_struct = malloc(sizeof(struct RQPacket));
	char * mode = "octect";
	pkt_struct->opCode = 1;
	if(type == 1)
		pkt_struct->opCode = htons(01);
	else if(type == 2)
		pkt_struct->opCode = htons(02);
	memcpy(pkt_struct->filename, filename, sizeof(&filename));
	pkt_struct->zb1 = '\0';
	memcpy(pkt_struct->mode, &mode, 6*sizeof(char));
	pkt_struct->zb2 = '\0';
	char * dpkt = (char *)(&pkt_struct);
	return dpkt;
}

char getOpcode(char * packet){
	char  opCode = 0;
	memcpy(&opCode, packet+1, 1 * sizeof(char));

	return opCode;
}

char * getFileName(char * packet){
	 char * firstNull = strchr(packet+2, '\0');
	 int fileLength = firstNull - (packet+2);
	 char * fileName = (char *) malloc(fileLength);
	 memcpy(fileName, packet+2, fileLength);
	 return fileName;
}