#ifndef SERVER_H
#pragma once
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <ws2tcpip.h>
#include <process.h>
#include <winsock.h>
#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tchar.h>
#include <direct.h>
#include <fstream>
#include <direct.h>

using namespace std;

//port data types

#define REQUEST_PORT 5001
#define	STKSIZE	 16536

// fill with server info, IP, port
union {struct sockaddr generic;
	struct sockaddr_in ca_in;}ca;

	char szbuffer[128];
	int calen=sizeof(ca); 

	//buffer data types

	char *buffer;
	int ibufferlen;
	int ibytesrecv;
	SOCKET s1;
	SOCKET s;
	int ibytessent;

	//host data types
	char localhost[11];

	HOSTENT *hp;

	//wait variables
	int nsa1;
	int r,infds=1, outfds=0;
	struct timeval timeout;
	const struct timeval *tp=&timeout;

	fd_set readfds;


#define HOSTNAME_LENGTH 20
#define RESP_LENGTH 40
#define FILENAME_LENGTH 20
#define BUFFER_LENGTH 128 
#define TRACE 0


	int num = 0;

typedef struct
{
	char filename[260];
	long filelenth;
}Fileinfo;

class TcpServer
{
	SOCKADDR_IN sa;      // filled by bind
	int port;

	public:
		TcpServer();
		~TcpServer();
		void TcpServer::start();
};

class Thread{
	public:
		Thread()
		{}
		virtual ~Thread()
		{}
		static void * pthread_callback (void * ptrThis);
		virtual void run () =0 ;
		void  start();
};

class TcpThread :public Thread
{         
	int cs;						 /* Socket descriptor */
	struct sockaddr_in ServAddr; /* server socket address */
	unsigned short ServPort;     /* server port */	
	WSADATA wsadata;
public:
	TcpThread(int clientsocket):cs(clientsocket)
	{}
	virtual void run();
	void listFiles(SOCKET ss);
	void PutFileToClient(SOCKET ps);
	void GetFileFromClient(SOCKET gs);
	void DeleteServerFiles(SOCKET ds);

};
	

#define SERVER_H
#endif