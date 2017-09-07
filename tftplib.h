#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>


#define TIMEOUT 3000
#define MAXPENDINGS 10
#define PORT 61005
#define MAXDATALENGTH 512
#define MAXPACKETLENGTH 2048
#define REQUESTHDR 10


char * createAckPacket(short blockNum);
char * createDataPacket(int blockNum, char * message, int size);

char * getFileName(char packet[]);
short getOpcode(char * packet);
short getBlockNumber(char * packet);
char * getDataPacket(char * packet, int size);
char * getErrorMessage(char * packet);
char * getErrorCode(char * packet);

void printPacket(char * packet, int size);
void printACKPacket(char * packet);
