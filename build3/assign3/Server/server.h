
#ifndef SER_TCP_H
#define SER_TCP_H


#include "Thread.h"

const char *FILE_DIR_ROOT = "../server_files_root/";

class SockServer: public SockLib {

public:
	SockServer(){};
	~SockServer(){};
	int start();

	// udp
	void client_handler();
	int recv_data(MSGHEADER &header, MSGREQUEST &request);
	int handshake();
};

// class TcpThread: public Thread {
// 	int cs;
// 
// public:
// 	TcpThread(int clientsocket) :
// 			cs(clientsocket) {
// 	}
// 	~TcpThread();
// 	virtual void run();
// 	int msg_recv(int sock, char *buf, int length);
// 	int msg_send(int sock, char *buf, int length);
// 
// 	int recv_data(MSGHEADER &header, MSGREQUEST &request);
// };

#endif
