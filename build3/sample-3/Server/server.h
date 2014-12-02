
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


