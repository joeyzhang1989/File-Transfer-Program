#ifndef SERVER_H
#define SERVER_H

//port data types

#define IOC_VENDOR 0x18000000
#define _WSAIOW(x,y) (IOC_IN|(x)|(y))
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)
#define REQUEST_PORT 5001
#define	STKSIZE	 16536
#define BUFFER_LENGTH 260 
#define PACKET_LENGTH 130
#define USEC 300000
#define SECT 0 


#pragma once
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include "../Common/syslogger.h"
#include <ws2tcpip.h>
#include <process.h>
#include <winsock.h>
#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <ctime>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tchar.h>
#include <direct.h>
#include <sstream>

using namespace std;


// fill with server info, IP, port
union {
	struct sockaddr generic;
	struct sockaddr_in ca_in;
}ca;

char szbuffer[BUFFER_LENGTH];
int calen = sizeof(ca);
char *buffer;
int ibufferlen;
int ibytesrecv;
SOCKET s1;
SOCKET s;
int ibytessent;
char localhost[11];
HOSTENT *hp;
int r, infds = 1, outfds = 0;
struct timeval timeout;
const struct timeval *tp = &timeout;
fd_set readfds;
//	int num = 0; test multithread

typedef struct
{
	char filename[260];
	long filelenth;
}Fileinfo;

typedef struct
{
	int seqNo;
	char data[BUFFER_LENGTH];
}Udppacket;


class TcpServer
{
	int cs;						 /* Socket descriptor */
	struct sockaddr_in sa; /* server socket address */
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
	Udppacket recvmsg;
	Udppacket sendmsg2;
	Udppacket recvmsg2;
	Udppacket nullmsg;
	int ibytesrecv;
	int ibytesput;
	int ibytesrecv2;
	int ibytesput2;
	string ack;
	string nak;
	string ack2;
	string nak2;
	int Send_temp_Seq;
	int rec_temp_seq;
	struct timeval timeout;

public:
	TcpServer();
	virtual void run();
	void listFiles(SOCKET ss);
	void PutFileToClient(SOCKET ps);
	void GetFileFromClient(SOCKET gs);
	void delFiles(SOCKET ds);
	void threewayhandshake();
	char* udpsend(SOCKET sock, char *sbuffer, int buflength, int sendflag);
	char* udprecv(SOCKET sock, char *sbuffer, int buflength, int sendflag);
	Udppacket udplastsend(SOCKET sock, char *sbuffer, int buflength, int sendflag);
	Udppacket udplastrecv(SOCKET sock, char *sbuffer, int buflength, int sendflag);
	~TcpServer();
};
#endif 