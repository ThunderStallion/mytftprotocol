#define MAXFILENAMESIZE 128
#define MAXERRORSIZE 128
#define DATASIZE 512


struct RQPacket{
	short opCode;
	char filename[MAXFILENAMESIZE];
	char zb1;
	char mode[8];
	char zb2;
};

struct DATAPacket{
	short opCode;
	short block_num;
	char data[DATASIZE];
};

struct ERRPacket{
	short opCode;
	short errorCode;
	char ErrorMessage[MAXERRORSIZE];
	char empty;
};
