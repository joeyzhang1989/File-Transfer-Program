
#ifndef __TCPLIB_H__
#define __TCPLIB_H__

#include <windows.h>
#include <winsock.h>

#define MAXPENDING 10
#define DEFAULT_WINDOWSIZE	8

class SockLib {
protected:
	int sock; 		// client socket or server listening socket
	int client_sock; // sockets that accepted by server

	struct sockaddr_in ServerAddr; 		/* server socket address */
	struct sockaddr_in ClientAddr;

	//unsigned short ServPort; /* server port */
	WSADATA wsadata;

	char hostname[HOSTNAME_LENGTH];

	// for UDP
	struct sockaddr_in dstAddr;		// 
	struct sockaddr_in localAddr;
	// sequence num
	unsigned int seq;
	unsigned int lastSeq;		// used by receiver to check if the frame is the same as last one

	// statistics
	int reSendCnt;		// resend times
	int sendCnt;		// send times not including resend times
	int recvCnt;
	int totalFramesSent;	// the total frames sent for a file.
	int fileSize;			// size of the file.

	bool showFile;		// print file content on the screen

	HANDSHAKE hsData;

	// assignment 3	
	int wSize;			// window size	
	char *winBuf;
	int winPos;			// window position
	int winBufPos;		// the position of window buffer
	unsigned int seqOfLastACK;

public:
	SockLib() {
	}
	~SockLib();

	int init();
	int client_init(const char *servername);
	int server_init();
	//void server_start();

	unsigned long resolve_name(const char *name);

	int sock_send(int sock, char *buf, int length);
	int sock_recv(int sock, char *buf, int length);

	int send_file(int sock, const char *filename, int len);
	int recv_file(int sock, const char *filename, int len);

	// udp
	int set_dstAddr(const char *dstHostName, int dstPort);
	int udp_init(int localPort, int windowSize);
	int sock_sendto(int sock, char *buf, int length, int handshake = 0);
	int sock_sendtoEx(int sock, char *buf, int length, int flag = 0);
	//int sock_recvfromEx(int sock, char *buf, int length, int handshake = 0);
	int sock_recvfrom(int sock, char *buf, int length, int handshake = 0);
	
protected:
	int lib_sendto(int sock, char *buf, int length);
	int udp_sendto(int sock, char *buf, int length, int handshake = 0);
	int udp_sendtoEx(int sock, char *buf, int length);
	int add_udpheader(PUDPPACKET pudp, char *buf);
	int lib_recvfrom(int sock, char *buf, int length, int handshake = 0);
	
	int send_ack(unsigned int seq, int type = ACKTYPE_REQUEST);
	int chk_seq(unsigned int seq);
	int chk_seqEx(PUDPPACKET pudp);
	
	int srv_wait4cnn(int sock, int sec = 0);

public:
	// statistics
	void reset_statistics(bool seq = false);
	void show_statistics(bool ifSend);

	//
	void setWindowsSize(int size);

protected:
	int copy_2_winbuf(const char *buf);
	int send_all_win_packets(int flag = 0);
	int remove_packets_from_win(unsigned int recvSeq, char arrAck[]);
	int set_last_seq(unsigned int &seqOfLastACK, PUDPPACKET pudp);
	int get_max_seq(unsigned int &retSeq, PUDPPACKET pudp, char arrAck[]);
	int send_special_request();
};



#endif
