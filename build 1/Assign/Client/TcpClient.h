#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <iostream>
#include <string.h>
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tchar.h>
#include <direct.h>
#include <string>
#include <fstream>
// link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#define HOSTNAME_LENGTH 20
#define RESP_LENGTH 40
#define REQUEST_PORT 5001
#define BUFFER_LENGTH 128 
#define TRACE 0
#define MSGHDRSIZE 8 //Message Header Size
#define LISTEN_PORT 5000 //Listen port to send/receive files to/from
#define FILENAME_LENGTH 260// the size is decided by the findfirst size of char name [260]


typedef struct
{
	char filename[FILENAME_LENGTH];
	long filelenth;
	
}ClientFile;
// the functions that can be executed by user
class TcpClient
{
private:
    int sock;                    /* Socket descriptor */
	struct sockaddr_in ServAddr; /* server socket address */
	unsigned short ServPort;     /* server port */	
	
public:
	TcpClient(){}
	boolean sendRequestPut();
	void connectServer(char remotehost[]);
	void listClientFiles();
	void listServerFiles();
	void deleteClientFiles();
	void GetFileFromServer();
	void PutFileToServer();
	void closeconnect();
	void deleteServerFiles();
	~TcpClient();
};



#endif // !FUNCTION_H

