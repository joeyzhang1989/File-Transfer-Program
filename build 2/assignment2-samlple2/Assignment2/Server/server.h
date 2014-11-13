#ifndef SERVER_H
#define SERVER_H

#include <tchar.h>
#include "tools.h"

class Server
{
private:
	SOCKET s;
	SOCKET s1;
	SOCKADDR_IN sa;      // filled by bind
	SOCKADDR_IN sa1;     // fill with server info, IP, port

	int calen;
	//buffer data types
	char szbuffer[512];
	char fileSzbuffer[512];

	int ibytesrecv;
	int ibytessent;
	int client_length;
	string cli_file;
	string cli_dir;
	string cli_size;
	//host data types
	char localhost[11];
	HOSTENT *hp;
	//wait variables
	int nsa1;
	int r;
	int infds;
	int outfds;
	int client_Start_Seq;
	int server_Start_Seq;
	int prev_Recv_Seq;
	int current_Recv_Seq;
	int Send_Seq;
	int next_Recv_Seq;
	struct timeval timeout;
	const struct timeval *tp;
	fd_set readfds;
	//others
	HANDLE test;
	DWORD dwtest;
	stringstream out;

	char *fn;

	ofstream fout;
	string filePath;
	union
	{
		struct sockaddr generic;
		struct sockaddr_in ca_in;
	}ca;
public:
	Server()
	{
		calen = sizeof(ca);
		infds = 1;
		outfds = 0;
		tp = &timeout;
		fn = ".\\log2.txt";
		filePath = ".\\";
	}
	void receiveFile(string);
	void sendFile(string);
	bool threeWayHandShake();
	void list();
	void receiveControl();
	void run();
	void deleteFiles(string);
private:
	string getFileSize(string);
};
#endif