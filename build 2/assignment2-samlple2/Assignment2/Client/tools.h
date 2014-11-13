#ifndef TOOLS_H
#define TOOLS_H
#pragma comment(lib,"ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include<fstream>
#include <winsock.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>
#include<ctime>
#include<list>
#include <Winsock2.h>

using namespace std;

#define IOC_VENDOR 0x18000000
#define _WSAIOW(x,y) (IOC_IN|(x)|(y))
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)
//user defined port number
#define REQUEST_PORT 7000
#define SEND_PORT 5000
#define SIZE 512

#define USEC 300000
#define SECT 0 
#define TRACE 1
#define FILELENGTH 260
struct segment_msg{
	char data[SIZE];
	int seqNo;
};
#endif