#define MAXFILENAMESIZE 128
#define MAXERRORSIZE 128
#define DATASIZE 512
#define ACKSIZE 4
#define RQSIZE 516

/* Opcodes */
#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5

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

struct ACKPacket{
	short opCode;
	short block_num;
}
