#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>


#define TIMEOUT 600
#define MAXPENDINGS 10
#define PORT 61008
#define MAXSTRINGLENGTH 512;

char findOpcode(char packet[]);
char * getFileName(char packet[]);

int main(int argc, char **argv)
{
	char recBuffer[512];
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

	/*set up serverAddress structure*/
	memset(&serverAddress, 0 , sizeof(serverAddress)); //zero out structure
    serverAddress.sin_family = AF_INET; //AF_INET signifies IPv4 address family
    hp = gethostbyname("localhost");
	bcopy ( hp->h_addr, &(serverAddress.sin_addr.s_addr), hp->h_length);
    serverAddress.sin_port = htons(PORT); //local port

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