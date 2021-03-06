//#include "tftpserver.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char getOpcode(char* packet);
char * getFileName(char packet[]);

int main(int argc, char** argv){
	
	char* packet = "01myfileName.txt\0mode\0";

	char * filename = getFileName(packet);
	char opcode = getOpcode(packet);
	printf("%c\n",opcode);
	printf("%s\n",filename);

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