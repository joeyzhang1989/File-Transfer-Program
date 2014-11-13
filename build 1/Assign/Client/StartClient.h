#ifndef STARTCLIENT_H // THE HEADER FILE FOR CLIENT
#define STARTCLIENT_H

#include <winsock2.h>
#include <stdio.h>
#include <iostream>
HOSTENT *localhostInfo,
		 *romotehostEntity;
#include <string>
//pre-define the values to be used by the parameters in the correspoding statements 
#define REMOTEHOST_LENGTH 10
#define COMMAND_LENGTH  20
// enum six components to capture the string commands,
// then return the predefined components to the swith statement for the function execution
enum string_code {
	first,
	second,
	third,
	forth,
	fifth,
	sixth,
	seventh,
};
char localhost[20];
#endif // !CLIENT_H
