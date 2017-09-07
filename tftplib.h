#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>


#define TIMEOUT 6000
#define MAXPENDINGS 10
#define PORT 61005
#define MAXDATALENGTH 512
#define MAXPACKETLENGTH 2048


char * createAckPacket(short blockNum);
char * createDataPacket(int blockNum, char * message, int size);

char * getFileName(char packet[]);
short getOpcode(char * packet);
short getBlockNumber(char * packet);
char * getDataPacket(char * packet, int size);
char * getErrorMessage(char * packet);

void printPacket(char * packet, int size);
void printACKPacket(char * packet);