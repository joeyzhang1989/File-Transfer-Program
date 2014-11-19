#ifndef UDP_H
#define UDP_H

#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <winsock.h>
#include <fstream>
#include <iostream>
#include <time.h>
#include<ctime>
#include <stdio.h>
#include <sstream>
#include <tchar.h>
using namespace std;

#define TRACE 1
#define MSG_SIZE 1024				//maximum packet size
#define DATA_SIZE (MSG_SIZE-2*sizeof(int))
#define DATA_SIZE2 (MSG_SIZE-3*sizeof(int))
#define ACK_DATA_SIZE (MSG_SIZE-sizeof(int)-sizeof(bool))
#define MAXHOSTNAMELEN 256			//maximum length of host name
#define TIMEOUT_USEC 300000			//time-out value
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)
#define WINDOW_SIZE 15
#define ACK true
#define NACK false

struct Message
{
	int seqNo1;
	int seqNo2;
	int validDataLength;
	char data[DATA_SIZE2];
	void disp()
	{
		cout<<"seqNo1:"<<seqNo1<<endl
			<<"seqNo2:"<<seqNo2<<endl;
	}
};

struct Event
{
	string command;
	string fileName;
	int size;
	void disp()
	{
		cout<<"command:"<<command<<endl
			<<"file name:"<<fileName<<endl
			<<"file size:"<<size<<endl;
	}
};

//this struct is for acknowledgement message
struct ACK_MSG
{
	bool flag;	//true for ACK, false for NACK
	int seq;
	char data[ACK_DATA_SIZE];
	void disp()
	{
		cout<<( flag?"ACK":"NACK")<<endl
			<<"seq:"<<seq<<endl
			<<"data"<<data<<endl;
	}
};
//this struct is for sending the content of a file
struct MESSAGE
{
	int seq;
	int validSize;
	char data[DATA_SIZE];
	void disp()
	{
		cout<<"seq:"<<seq<<endl
			<<"validSize"<<validSize<<endl
			<<"data"<<data<<endl;
	}
};

class UDP
{
private:
	int requestPort;
	int sendPort;
	char localhost[MAXHOSTNAMELEN];
	ofstream fout;					//log file
	SOCKET mySocket;				//socket used for communcation with remote host
	SOCKADDR_IN sa_in_local;		// address structure for local host address
	SOCKADDR_IN sa_in_remote;		// address structure for remote host address
	struct timeval timeout;
	struct timeval timeout_flag;
	int remoteSeq;
	int localSeq;
	Event event;
	
public:
	UDP(int,int);
	void run();
	~UDP();
private:
	bool threeWayHandShake();
	void sendPacket(MESSAGE &);
	void sendPacket(ACK_MSG &);
	void sendPacket(Message &);
	void getPacket(MESSAGE &);
	void getPacket(ACK_MSG &);
	void getPacket(Message &);
	int getFileSize(string);
	string intToString(int);
	int stringToInt(string);
	string list();
	void sendList(string);
	DWORD setBlock(bool);
	void recieveFile();
	void sendFile();
	void guaranteedSendPacket(Message &, Message &);
};


#endif