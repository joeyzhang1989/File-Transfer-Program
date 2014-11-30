
#include <winsock.h>
#include <iostream>
#include <windows.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <process.h>
#include <string>
#include <exception>

#include "../common/syslogger.h"
#include "../common/protocol.h"
#include "../common/socklib.h"
#include "server.h"


using namespace std;


int SockServer::handshake() {
	HANDSHAKE hs;
	int	ret = -1;

	// wait for client's request
	hsData.clientSeq = 0;
	hsData.serverSeq = rand();
	ret = sock_recvfrom(sock, (char *)&hs, sizeof(HANDSHAKE), 1);
	if (ret) {
		SysLogger::inst()->err("failed to get handshake request.");
		return -1;
	}

	// wait for client's response
	// 	hs.clientSeq = 0;
	// 	hs.serverSeq = 0;
	// 	ret = sock_recvfrom(sock, (char *)&hs, sizeof(HANDSHAKE), 1);
	// 	if (ret) {
	// 		SysLogger::inst()->err("failed to get handshake response.");
	// 		return -1;
	// 	}
	// handshake OK.
	// save the client's sequence number.
	//srv_wait4cnn(sock, 10);		// make sure that the ACK of last packet sent

	seq = hs.clientSeq;
	reset_statistics(true);

	return 0;
}

int SockServer::start() {

	bool ifHandShake = true;
	while (1) {
		if (ifHandShake) {
			if (handshake()) {
				return -1;
			}
			ifHandShake = false;
			continue;
		}
		client_handler();
	}
	return 0;
}

int SockServer::recv_data(MSGHEADER &header, MSGREQUEST &request) {
	// began to receive the request header
	string type;

	if (sock_recvfrom(sock, (char *)&header, sizeof(MSGHEADER))) {
		SysLogger::inst()->err("failed to get header of request");
		return MSGTYPE_RESP_FAILTOGETHEADER;
	}
	header.len = ntohl(header.len);
	if (header.type == MSGTYPE_REQ_GET) {
		type = MSGTYPE_STRGET;
		if (header.len != sizeof(MSGREQUEST)) {
			SysLogger::inst()->err("header.len != sizeof(request). %d, %d", header.len, sizeof(MSGREQUEST));
			return MSGTYPE_RESP_WRONGHEADER;
		}
	}
	else if (header.type == MSGTYPE_REQ_PUT) {
		type = MSGTYPE_STRPUT;
	}
	else {
		SysLogger::inst()->err("unknown request type");
		return MSGTYPE_RESP_UNKNOWNTYPE;
	}
	SysLogger::inst()->log("Received a request(type: %s, len: %d)", type.c_str(), header.len);

	// get the filename and host name
	if (header.len > 0) {
		if (sock_recvfrom(sock, (char *)&request, sizeof(MSGREQUEST))) {
			SysLogger::inst()->err("failed to get request info.");
			return MSGTYPE_RESP_FAILTOGETINFO;
		}
		SysLogger::inst()->log("hostname: %s, filename: %s", request.hostname, request.filename);
	}
	if (header.type == MSGTYPE_REQ_PUT) {
		SysLogger::inst()->out("User \"%s\" requested file %s to be Received.",  request.filename);
	}
	else {
		SysLogger::inst()->out("User \"%s\" requested file %s to be sent.",  request.filename);
	}

	// send back the response
	string filename = FILE_DIR_ROOT;
	filename += request.filename;

	if (header.type == MSGTYPE_REQ_PUT) {
		// continue to receive the file before sending response
		SysLogger::inst()->out("Receiving file from %s, waiting...", request.hostname);
		if (SockLib::recv_file(sock, filename.c_str(), header.len - sizeof(MSGREQUEST))) {
			return MSGTYPE_RESP_FAILTORECVFILE;
		}
		srv_wait4cnn(sock, 25);		// make sure that the ACK of last packet sent
		SysLogger::inst()->out("Successfully receive the file: %s", request.filename);
	}

	return MSGTYPE_RESP_OK;
}

void SockServer::client_handler() {
	MSGHEADER header;
	MSGREQUEST request;
	MSGHEADER header_resp;

	// handle request
	SysLogger::inst()->out("Receiving request...");
	memset((void *)&header, 0, sizeof(MSGHEADER));
	memset((void *)&request, 0, sizeof(MSGREQUEST));
	header_resp.type = recv_data(header, request);

	// send back the response
	header_resp.len = 0;
	string filename = FILE_DIR_ROOT;
	filename += request.filename;

	if (header.type == MSGTYPE_REQ_GET) {
		// get the file size
		FILE *pFile = 0;

		pFile = fopen(filename.c_str(), "rb");
		if (pFile == NULL) {
			SysLogger::inst()->err("No such a file: %s\n", filename.c_str());
			header_resp.type = MSGTYPE_RESP_NOFILE;
		}
		else {
			fseek(pFile, 0, SEEK_END);
			header_resp.len = ftell(pFile);
			fileSize = header_resp.len;
			fclose(pFile);
		}
	}
	show_statistics(false);

	// send header
	SysLogger::inst()->out("Sending response...");
	if (sock_sendtoEx(sock, (char *)&header_resp, sizeof(header_resp)) != 0) {
		SysLogger::inst()->err("sock_sendto error. header.type:%d\n", header.type);
		return;
	}
	SysLogger::inst()->log("Send response: header.type: %d, len: %d", header_resp.type, header_resp.len);

	// send file
	if (header.type == MSGTYPE_REQ_GET && header_resp.type == MSGTYPE_RESP_OK) {
		SysLogger::inst()->out("Sending file to %s, waiting...", request.hostname);
		if (send_file(sock, filename.c_str(), header_resp.len)) {
			return;
		}
		SysLogger::inst()->out("Successfully send the file: %s", request.filename);
	}
	SysLogger::inst()->log("Send response: file: %s ", filename.c_str());
	show_statistics(true);
	SysLogger::inst()->out("\n");
}


int main(void) {
	srand(time(NULL));

	// create logger
	if (SysLogger::inst()->set("../logs/server_log.txt")) {
		return -1;
	}
	SysLogger::inst()->wellcome();

	SysLogger::inst()->out("Please set the window size: ");
	int windowSize = DEFAULT_WINDOWSIZE;

	//cin >> windowSize;

	SockServer *ts = new SockServer();

	try {
		ts->setWindowsSize(8);
		if (ts->udp_init(SERVER_RECV_PORT, windowSize)) {
			delete ts;
			return -1;
		}

		if (ts->start()) {
			delete ts;
			return -1;
		}
	}
	catch (int e) {
		e = 0;
		delete ts;
		return -1;
	}

	delete ts;
	return 0;
}

