#ifndef CLIENT_H
#define CLIENT_H
#include <winsock.h>
#include <stdio.h>

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <time.h>
#include <fstream>
#pragma comment(lib,"wsock32.lib") //for linker

//User defined port number
#define REQUEST_PORT 7000;
#define LOCAL_PORT 5000;

#define ROUTER_PORT1 7000			//router port number 1
#define ROUTER_PORT2 7001			//router port number 2
#define PEER_PORT1 5000				//port number of peer host 1
#define PEER_PORT2 5001				//port number of peer host 2

int port=REQUEST_PORT;
int lport = LOCAL_PORT;

//socket data types
SOCKET s;
SOCKET sr;
SOCKADDR_IN clientAddr;			// filled with client info, IP, port
SOCKADDR_IN serverAddr;			// filled with server info, IP, port
int calen = sizeof(clientAddr);
int k = 4;
const char SWS = '3';
const int dataSize = 1038;
const int packetSize = 1048;

//buffer data types
char packet[packetSize];	//file packet
char client[packetSize];	//client information packet
char flag[2];				//signal between client and server
char ack[8];				//contains ACK or NAK

const int windowSize = SWS;
char windowBuffer[windowSize][packetSize];
int ibufferlen;
int ibytesrecv;
int ibytessent;
char *buffer;

int bytessent=0;
int bytesrecv=0;

//host data types
HOSTENT *hp;
HOSTENT *rp;

char localhost[21];			//local host name
char remotehost[21];		//remote host name
char direction[4];			//file transfer direction
char* clogFile = "client_log.log"; //client log file
int r = 2;			
int infds=1, outfds=0;
struct timeval timeout;
fd_set readfds;

//other
HANDLE test;
DWORD dwtest;
char cusername[128];   // Other header data
DWORD dwusername = sizeof(cusername);               // Retains the size of the username
// Structure to hold client request info
struct client_info {
	char filename[35];		//source file name
	char username[21];		//client username
	char direction[4];
	char localhost[21];		//transfer direction
	int dropRate;			//network drop r
};

// Structure of packet 
struct packet_data {
	char wSeq;				//sequence number within a window
	int seqNumber;			//sequence number 5 bytes of the whole packets
	char dataBuffer[dataSize];	//80-byte data buffer
};

//Structure of ACK
struct ack_data {
	char type;				//flag type: 'a' means ACK, 'n' means NAK
	char wSeq;				//window sequence number of acknowledged packet
	int seqNumber;			//sequence number of acknowledged packet
};

/*--------------------------*/
/*--------functions-------- */
/*--------------------------*/

//sending flag			
int sendACK();
//receiving flag				
int receiveAck();
//geting destination file from server
int get(char file[]);
//sending source to server	
int put(char file[]);	
//sending signal to server	
int sendFlag();		
//receiving signal from server		
int receiveFlag();			
//sending the window
void sendWindow(int, int);
//random number generator
bool sendwindow();		
//getting the length of the packet	
int getPacketLength(char* buffer);
//buffer cleaning
void clearbuf(char* buf, int size) 
{
	for (int i=0; i<size; i++) 
		buf[i] = 0;
}
#endif 
