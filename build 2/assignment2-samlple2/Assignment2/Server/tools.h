#ifndef TOOLS_H
#define TOOLS_H
#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <winsock.h>
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <Winsock2.h>
#include<direct.h>
#include <io.h>
using namespace std;

#define IOC_VENDOR 0x18000000
#define _WSAIOW(x,y) (IOC_IN|(x)|(y))
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)
#define REQUEST_PORT 0x7070
#define USEC 300000
#define SECT 0 
#define TRACE 1
#define SEND_PORT 5001
#define BUFFER_SIZE 512
#define FILELENGTH 260

struct segment_msg{
	char data[512];
	int seqNo;
};

#endif