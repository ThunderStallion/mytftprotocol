//#include "tftpserver.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>

char  getOpcode(char* packet);
char * getFileName(char * packet);
char * getErrorMessage(char * packet);
int main(int argc, char** argv){
	
	short opcodet = htons(03);
	char filenamez[9] = "heymanplz";
	char zb1 = '\0';
	char mode[6] = "octect";
	char zb2 = '\0';
	char * packet = malloc(19 * sizeof(char));
	memcpy(packet, &opcodet, 2);
	memcpy(packet+2, &filenamez, 9);
	memcpy(packet+11, &zb1, 1);
	memcpy(packet+12,&mode, 6);
	memcpy(packet+18,&zb2,1);



	char * filename = getFileName(packet);
	printf("%s\n",filename);
	free(filename);
	char opcode = getOpcode(packet);
	printf("%d\n",opcode);


	short TTopcodet = htons(05);
	short TTerrCode = htons(02);
	char TTerrMsg[12] = "Invalid File";
	char TTzb = '\0';
	char * packet2 = malloc(17 * sizeof(char));
	memset(packet2, 0,(17 * sizeof(char)) );
	memcpy(packet2, &TTopcodet, 2);
	memcpy(packet2+2, &TTerrCode, 2);
	memcpy(packet2+4, &TTerrMsg, 12);
	memcpy(packet2+16,&TTzb, 1);

	for(int x= 0 ; x<17; x++){
		printf("[%d]: %d\n", x, packet2[x]);
	}

	char * message = getErrorMessage(packet2);
	printf("%s\n", message);
	return 0;


}

char * getErrorMessage(char * packet){
	char * firstNull = strchr(packet+4,'\0');
	int errMsgLen = firstNull - (packet+4);
	printf("%d\n", errMsgLen);
	char * errMsg = (char *) malloc(errMsgLen);
	memcpy(errMsg, packet+4, errMsgLen);
	return errMsg;
}

char  getOpcode(char * packet){
	char  opCode = '0';
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