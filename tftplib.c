#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "tftplib.h"

char * createAckPacket(short blockNum){
	char * pkt_ptr = malloc(4);
	short opCode = htons(04);
	short blockNumber = htons(blockNum);
	memcpy(pkt_ptr, &opCode , 2);
	memcpy(pkt_ptr+2, &blockNumber , 2);
	return pkt_ptr;
}

char * createDataPacket(int blockNum, char * message, int size){
	printf("[Server] Creating Data Packet\n");

	int length = size+4;
	char * pkt_ptr = malloc(length);
	short opCode = htons(03);

	memcpy(pkt_ptr, &opCode , 2);
	memcpy(pkt_ptr+2, &blockNum, 2);
	memcpy(pkt_ptr+4, message , size);
	return pkt_ptr;
}

short getOpcode(char * packet){
	short  opCode = 0;
	memcpy(&opCode, packet, 1 * sizeof(short));
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

char * getErrorMessage(char * packet){
	char * firstNull = strchr(packet+4,'\0');
	int errMsgLen = firstNull - (packet+4);
	printf("%d\n", errMsgLen);
	char * errMsg = (char *) malloc(errMsgLen);
	memcpy(errMsg, packet+4, errMsgLen);
	return errMsg;
}

char * getErrorCode(char * packet) {
	char * code = malloc(2);
	memcpy(code, packet+2, 2);
	return code;
}

void printPacket(char * packet, int size){
	for(int x=0 ; x<2; x++){
		printf("[%d]: %d\n", x, packet[x]);
	}
	for(int x= 2 ; x<size; x++){
		printf("[%d]: %c\n", x, packet[x]);
	}
}

void printACKPacket(char * packet){
	for(int x=0 ; x<2; x++){
		printf("[%d]: %d\n", x, packet[x]);
	}
	for(int x= 2 ; x<4; x++){
		printf("[%d]: %d\n", x, packet[x]);
	}
}


char * getFileName(char * packet){
	 char * firstNull = strchr(packet+2, '\0');
	 int fileLength = firstNull - (packet+2);
	 char * fileName = (char *) malloc(fileLength);
	 memcpy(fileName, packet+2, fileLength);
	 return fileName;
}
