
#include <stdio.h>
#include <iostream>
#include <string>
#include <time.h>
#include <windows.h>

#include "../common/syslogger.h"
#include "../common/protocol.h"
#include "../common/socklib.h"
#include "client.h"
#include <direct.h>
#include <io.h>
using namespace std;


int SockClient::handshake() {
	int	ret = -1;
	HANDSHAKE hs;

	hsData.clientSeq = 0;
	hsData.serverSeq = 0;
	hs.serverSeq = 0;
	hs.clientSeq = rand();			// TODO: htonl, ntohl

	// send hand shake request
	SysLogger::inst()->out("Sending Handshake Request...");

	ret = sock_sendto(sock, (char *)&hs, sizeof(HANDSHAKE), 1);
	if (ret) {
		SysLogger::inst()->err("failed to send handshake request.");
		return -1;
	}
	SysLogger::inst()->out("Sent a Handshake Request (%d, %d)", hs.clientSeq, hs.serverSeq);

	// wait for server's response
	if (hsData.clientSeq != hs.clientSeq) {
		SysLogger::inst()->err("failed to get handshake response (%d, %d)", hsData.clientSeq, hsData.serverSeq);
		return -1;
	}
	SysLogger::inst()->out("Received a Handshake Response (%d, %d) (%d, %d)",
		hsData.clientSeq, hsData.serverSeq,
		hsData.clientSeq & SEQUENCE_NUM_MASK, hsData.serverSeq & SEQUENCE_NUM_MASK);

	// send hand shake response
	// client may fail to send this frame as the high loss rate of the router.
	// so we need an ACK for this frame
	hs.serverSeq = hsData.serverSeq;
	// 	ret = sock_sendto(sock, (char *)&hs, sizeof(HANDSHAKE), 1);
	// 	if (ret) {
	// 		SysLogger::inst()->err("failed to send handshake response.");
	// 		return -1;
	// 	}
	// 	SysLogger::inst()->out("Sent a Handshake Response (%d, %d)\n", hs.clientSeq, hs.serverSeq);
	// 
	// 	if (hs.clientSeq != hsData.clientSeq || hs.serverSeq != hsData.serverSeq) {
	// 		SysLogger::inst()->err("failed to get the last Handshake ACK (%d, %d)", hsData.clientSeq, hsData.serverSeq);
	// 		return -1;
	// 	}

	// handshake OK.
	// save the server's sequence number.
	seq = hs.serverSeq;
	reset_statistics(true);

	return 0;
}

//----------------------------------------------------
//Get the file list of the localhost
//----------------------------------------------------

void listFiles()

{
	struct _finddata_t clientfile;
	long clientfileHandle;
	char clientdirbuffer[256];
	TCHAR infoBuf[256];
	SetCurrentDirectory("C:\\Users\\chenglong\\Desktop\\assign3\\Server");
	_getcwd(clientdirbuffer, 256);
	GetWindowsDirectory(infoBuf, 256);
	printf("Your Windows directory is: %S\n", infoBuf);
	//string curPath";
	string curPath = strcat(clientdirbuffer, "\\*.*");


	if ((clientfileHandle = _findfirst(curPath.c_str(), &clientfile)) == -1)
	{
		return;
	}
	printf("Server File List\n");
	printf("********************************\n");
	do {
		if (_A_ARCH == clientfile.attrib)
		{
			printf("%s\n", clientfile.name);
		}
	} while (!(_findnext(clientfileHandle, &clientfile)));
	printf("********************************\n");
	_findclose(clientfileHandle);
}
int SockClient::start(const char *filename, const char *opname) {
	if (filename == 0 || opname == 0) {
		SysLogger::inst()->err("msg_send params error");
		return -1;
	}

	// hand shake
	static bool ifHandShake = true;
	if (ifHandShake) {
		if (handshake()) {
			return -1;
		}
		ifHandShake = false;
	}

	SysLogger::inst()->out("Sending request...");
	// create the header of msg
	MSGHEADER header;
	MSGREQUEST request;
	string filefullname = FILE_DIR_ROOT;
	filefullname += filename;

	memset((void *)&header, 0, sizeof(MSGHEADER));
	memset((void *)&request, 0, sizeof(MSGREQUEST));
	header.len = sizeof(request);
	if (strcmp(opname, MSGTYPE_STRGET) == 0)
		header.type = MSGTYPE_REQ_GET;
	else if (strcmp(opname, MSGTYPE_STRPUT) == 0) {
		header.type = MSGTYPE_REQ_PUT;

		//read the size of file to be sent to server
		FILE *pFile = 0;

		pFile = fopen(filefullname.c_str(), "rb");
		if (pFile == NULL) {
			SysLogger::inst()->err("No such a file:%s\n", filefullname.c_str());
			return -1;
		}
		fseek(pFile, 0, SEEK_END);
		fileSize = ftell(pFile);
		header.len += fileSize;

		fclose(pFile);
	}
	else {
		SysLogger::inst()->err("Wrong request type\n");
		return -1;
	}

	//send out the header + filename + hostname
	header.len = htonl(header.len);
	if (sock_sendtoEx(sock, (char *)&header, sizeof(header)) != 0) {
		SysLogger::inst()->err("sock_send error. header.type: %d, len: %d\n", header.type, ntohl(header.len));
		return -1;
	}
	memmove(request.filename, filename, strlen(filename));
	memmove(request.hostname, hostname, strlen(hostname));
	if (sock_sendtoEx(sock, (char *)&request, sizeof(request)) != 0) {
		SysLogger::inst()->err("sock_send error. filename: %s, hostname: %s\n",
			request.filename, request.hostname);
		return -1;
	}

	if (header.type == MSGTYPE_REQ_PUT) {
		// send file to server
		if (SockLib::send_file(sock, filefullname.c_str(), header.len - sizeof(request))) {
			return -1;
		}
	}
	show_statistics(true);

	//receive the response, first get the header
	SysLogger::inst()->out("Receiving response...");
	MSGHEADER header_resp;
	if (sock_recvfrom(sock, (char *)&header_resp, sizeof(header_resp))) {
		SysLogger::inst()->err("failed to get header of response");
		return -1;
	}

	if (header_resp.type != MSGTYPE_RESP_OK) {
		const char *ERROR_MSG[] = {
			"NULL",
			"Fail to receive the request header",
			"Wrong request header",
			"Unknown request type",
			"Fail to receive the request data",
			"Fail to receive the file",
			"No such a file",
		};

		SysLogger::inst()->err("Response ERROR: %d. %s", header_resp.type, ERROR_MSG[header_resp.type]);
		return -1;
	}

	SysLogger::inst()->out("Get an OK response, data length: %d", header_resp.len);

	// get the file from server
	if (header_resp.len > 0) {
		if (SockLib::recv_file(sock, filefullname.c_str(), header_resp.len)) {
			return -1;
		}
		SysLogger::inst()->out("Received a file: %s", filefullname.c_str());
	}

	srv_wait4cnn(sock, 2);		// make sure that the ACK of last packet sent

	show_statistics(false);
	return 0;
}

int main(int argc, char *argv[]) {
	srand(time(NULL));

	// create logger
	if (SysLogger::inst()->set("../logs/client_log.txt")) {
		return -1;
	}
	SysLogger::inst()->wellcome();

	SysLogger::inst()->out("Please set the window size: ");
	int windowSize = DEFAULT_WINDOWSIZE;

	//cin >> windowSize;

	//get input
	string servername, filename, opname = "";

	while (1) {
		SysLogger::inst()->out("\nType name of ftp server (router): ");
		servername = "";
		filename = "";
		opname = "";

		cin >> servername;
		if (servername == "quit") {
			break;
		}

		SysLogger::inst()->out("Type name of file to be transferred: ");
		cin >> filename;
		SysLogger::inst()->out("Type direction of transfer: ");
		cin >> opname;
		
		//start to connect to the server
		SockClient * tc = new SockClient();

		if (tc->udp_init(CLIENT_RECV_PORT, windowSize) == 0) {
			if (tc->set_dstAddr(servername.c_str(), CLIENT_DST_RECV_PORT) == 0) {
				SysLogger::inst()->out("\nSent request to %s, waiting...\n", servername.c_str());
				while (true){
					if (tc->start(filename.c_str(), opname.c_str())) {
						//listFiles();
						break;
					}

				}
				
			}
		}
		delete tc;

		//cin >> opname;
	}

	return 0;
}
