
#ifndef CLIENT_H
#define CLIENT_H
#include <tchar.h>
#include "tools.h"
#include<direct.h>
#include <io.h>
class Client
{
private:
	SOCKET mySocket;
	SOCKADDR_IN sa;         // filled by bind
	SOCKADDR_IN sa_in;      // fill with server info, IP, port
	char szbuffer[SIZE];
	int server_length;
	int client_Start_Seq;
	int server_Start_Seq;
	int prev_Recv_Seq;
	int current_Recv_Seq;
	int Send_Seq;
	int next_Rec_Seq;
	char localhost[21];
	char remotehost[21];

	HANDLE test;
	DWORD dwtest;

	char sz[SIZE];
	char fileSz[SIZE];

	struct timeval timeout;
	struct timeval timeout1;
	char *fn;
	ofstream fout;
	int outfds;
	string filePath;
	string serv_size;
public:
	Client()
	{
		fn = ".\\log.txt";
		outfds = 0;
		filePath = ".\\";
	}
	void receiveFile(string);
	void sendFile(string);
	bool threeWayHandShake();
	void listClientFiles();
	void listServerFiles();
	void sendControl(string, string);
	void run();
	void deleteClientFiles();
	void deleteServerFiles();
	~Client(){};
private:
	string getFileSize(string);

};


#endif