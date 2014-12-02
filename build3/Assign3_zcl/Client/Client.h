#ifndef CLIENT_H
#define CLIENT_H
#define IOC_VENDOR 0x18000000
#define _WSAIOW(x,y) (IOC_IN|(x)|(y))
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)
#define REQUEST_PORT 7000
#define BUFFER_LENGTH 260 
#define PACKET_LENGTH 130
#define LISTEN_PORT 5000 //Listen port to send/receive files to/from
#define USEC 300000
#define SECT 0 

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <iostream>
#include <string.h>
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tchar.h>
#include <ctime>
#include <direct.h>
#include <sstream>
#include "../Common/syslogger.h"
#pragma comment( linker, "/defaultlib:ws2_32.lib" )

using namespace std;

typedef struct
{
	int seqNo;
	char data[BUFFER_LENGTH];
}Udppacket;

typedef struct
{
		char filename[260];
		int filelenth;
}Fileinfo;

class TcpClient
{
    int sock;                    /* Socket descriptor */
	struct sockaddr_in ServAddr; /* server socket address */
	struct sockaddr_in sa_in;
	unsigned short ServPort;     /* server port */	
	WSADATA wsadata;
	int client_Start_Seq;
	int server_Start_Seq;
	int prev_Recv_Seq;
	int next_Rec_Seq;
	int current_Recv_Seq;
	int Send_Seq;
	int totalsendnumberpackets;
	int totalsendpacket;
	int totalrecvnumberpackets;
	int totalrecvpacket;
	int outfds;
	int outfds2;
	Udppacket sendmsg;
	Udppacket sendmsg2;
	Udppacket recvmsg;
	Udppacket recvmsg2;
	Udppacket nullmsg;
	int ibytesrecv;
	int ibytesrecv2;
	int ibytesput;
	int ibytesput2;
	string ack;
	string ack2;
	string nak;
	string nak2;
	struct timeval timeout;
	int Send_temp_Seq;
	int rec_temp_seq;

public:
	TcpClient(){};
	void connectserver(char servername[]);
	void listClientFiles();
	void listServerFiles();
	void DeleteServerFiles();
	void GetFileFromServer();
	void PutFileToServer();
	void closeconnect();
	void setsain(char servername[]);
	bool threewayhandshake();
	char* udpsend(SOCKET sock,char *sbuffer,int buflength, int sendflag);
	char* udprecv(SOCKET sock,char *sbuffer,int buflength, int sendflag);
	Udppacket udplastsend(SOCKET sock,char *sbuffer,int buflength, int sendflag);
	Udppacket udplastrecv(SOCKET sock,char *sbuffer,int buflength, int sendflag);
	void log();
	~TcpClient();
};

#endif 