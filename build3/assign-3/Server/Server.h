#ifndef SERVER_H
#define SERVER_H
#include <io.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winsock.h>
#include <time.h>
#include <fstream>
#pragma comment(lib,"wsock32.lib") //for linker

#define ROUTER_PORT1 7000			//router port number 1
#define ROUTER_PORT2 7001			//router port number 2
#define PEER_PORT1 5000				//port number of peer host 1
#define PEER_PORT2 5001				//port number of peer host 2

//Port data types
#define REQUEST_PORT 0x7070
int port=REQUEST_PORT;

//socket data types
SOCKET s;
SOCKADDR_IN saddr;      // filled by bind
SOCKADDR_IN caddr;     // fill with server info, IP, port
union {
	struct sockaddr generic;
	struct sockaddr_in ca_in;
}ca;

const char SWS = '3';	
const int dataSize = 1038;
const int packetSize = 1048;

int calen=sizeof(ca); 
const char* logfile = "server_log.log";// server log file
int rate = 2;
int k = 4;
//buffer data types
	
char client_info[packetSize]; //for holding client infomation
char packet[packetSize]; //for holding file package
char flag[2];	//for verifing if the file exists 
char NackBuffer[8]; //for holding NAck structure
const int windowSize = SWS - 48;
char windowBuffer[windowSize][packetSize];
char *buffer;
int ibufferlen;
int ibytesrecv;	
int ibytessent;	
int caddrLen;

//host data types
char localhost[21];
HOSTENT *hp; //information for server
//HOSTENT *hs; //information for client

//wait variables
int nsa1;
int r,infds=1, outfds=0;
struct timeval timeout;
const struct timeval *tp=&timeout;
fd_set readfds;

//others
HANDLE test;
DWORD dwtest;

//Data structure for holding received data
struct received_data 
{
	char filename[35];
	char sender[11];
	char direction[4];
	char remotehost[21];
	int dropRate;
};

struct packet_data 
{
	char wSeq;
	int seqNumber;
	char dataBuffer[dataSize];
};

struct NAck
{
	// n->Nak a->Ack
	char index;
	//0 to 7
	char wSeq;  
	int seq;
};


#endif 