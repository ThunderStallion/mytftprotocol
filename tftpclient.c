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
	struct hostent *hp, *gethostbyname();
	

	/*initialize socket structure*/
	struct sockaddr_in clientAddress;  //Client address
	memset(&clientAddress, 0 , sizeof(clientAddress)); //zero out structure
    server.sin_family = AF_INET; //AF_INET signifies IPv4 address family
    hp = gethostbyname(argv[1]);
	bcopy ( hp->h_addr, &(server.sin_addr.s_addr), hp->h_length);
    server.sin_port = htons(PORT); //local port

    /*Create a socket */
	sock = socket(AF_INET,SOCK_DGRAM, 0);
	if (sock < 0){
		perror("Server: socket could not be created");
		exit(0);
	}
    if (bind(sock, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) < 0) {
		perror("bind failed");
		return 0;
	}       
    /*setting timeout struct*/
    struct timeval tv;
    tv.tv_sec=TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));


	for(;;){
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