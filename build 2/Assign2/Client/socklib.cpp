/*
   COMP6461 Assignment2

   Chenglong Zhang (ID: 6842666) 
   Liu Sun         (ID: 6758878) 
 
 * This file is created by Chenglong Zhang & Liu Sun
 * $Author$
 * $Date$
 * $Rev$
 * $HeadURL$
 *
 */


#include <stdio.h>

#include "syslogger.h"
#include "protocol.h"
#include "socklib.h"

#pragma comment(lib, "ws2_32.lib")


SockLib::~SockLib() {
	/* When done uninstall winsock.dll (WSACleanup()) and exit */
	closesocket(sock);
	WSACleanup();
	delete winBuf;
}

int SockLib::init() {
	//initilize winsocket
	if (WSAStartup(0x0202, &wsadata) != 0) {
		SysLogger::inst()->err("Error in starting WSAStartup()\n");
		return -1;
	}

	//Display name of local host and copy it to the req
	if (gethostname(hostname, HOSTNAME_LENGTH) != 0) {
		SysLogger::inst()->err("can not get the host name,program exit");
		//WSACleanup();
		return -1;
	}

	//Create the socket
	//if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		SysLogger::inst()->err("Socket Creating Error");
		//WSACleanup();
		return -1;
	}
	SysLogger::inst()->log("create a socket: %d", sock);

	return 0;
}

int SockLib::client_init(const char *servername) {
	if (servername == 0) {
		SysLogger::inst()->err("init params error");
		return -1;
	}
	
	if (init()) {
		SysLogger::inst()->err("socket init error");
		return -1;
	}

	SysLogger::inst()->out("ftp_tcp starting on host: [%s]", hostname);
	
	//connect to the server
	memset(&ServerAddr, 0, sizeof(ServerAddr)); /* Zero out structure */
	ServerAddr.sin_family = AF_INET; /* Internet address family */
	ServerAddr.sin_addr.s_addr = resolve_name(servername); /* Server IP address */
	ServerAddr.sin_port = htons(SERVER_RECV_PORT); /* Server port */
	if (connect(sock, (struct sockaddr *) &ServerAddr, sizeof(ServerAddr)) < 0) {
		SysLogger::inst()->err("Faild to connect to server: %s", servername);
		//closesocket(sock);
		//WSACleanup();
		return -1;
	}


	return 0;
}

int SockLib::server_init() {
	if (init()) {
		SysLogger::inst()->err("socket init error");
		return -1;
	}

	SysLogger::inst()->out("ftpd_tcp starting at host: [%s]", hostname);

	//Fill-in Server Port and Address info.
	memset(&ServerAddr, 0, sizeof(ServerAddr)); /* Zero out structure */
	ServerAddr.sin_family = AF_INET; /* Internet address family */
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	ServerAddr.sin_port = htons(SERVER_RECV_PORT); /* Local port */

	//Bind the server socket
	if (bind(sock, (struct sockaddr *) &ServerAddr, sizeof(ServerAddr))	== SOCKET_ERROR) {
		SysLogger::inst()->err("bind error");
		//closesocket(sock);
		//WSACleanup();
		return -1;
	}

	//Successfull bind, now listen for Server requests.
	if (listen(sock, MAXPENDING) == SOCKET_ERROR) {
		SysLogger::inst()->err("listen error");
		//closesocket(sock);
		//WSACleanup();
		return -1;
	}
	SysLogger::inst()->out("waiting to be contacted for transferring files...\n");

	return 0;
}

unsigned long SockLib::resolve_name(const char *name) {
	struct hostent *host; /* Structure containing host information */

	if ((host = gethostbyname(name)) == NULL) {
		SysLogger::inst()->err("gethostbyname() failed");
		return 0;
	}

	/* Return the binary, network byte ordered address */
	return *((unsigned long *) host->h_addr_list[0]);
}

int SockLib::sock_recv(int sock, char *buf, int length) {
	int ret = SOCKET_ERROR, left = length;
	char *p = buf;

	do {
		ret = recv(sock, p, left, 0);
		if (ret == 0) {
			SysLogger::inst()->err("msg_recv connection closed");
			return -1;
		} else if (ret < 0) {
			SysLogger::inst()->err("msg_recv recv error");
			return -1;
		}
		left -= ret;
		SysLogger::inst()->log("Recv %d bytes, left: %d", ret, left);
		p += ret;
		ret = SOCKET_ERROR;
	} while (left > 0);

	return left;
}

int SockLib::sock_send(int sock, char *buf, int length) {
	int ret = SOCKET_ERROR, left = length;
	char *p = buf;

	do {
		ret = send(sock, p, left, 0);
		if (ret == SOCKET_ERROR) {
			SysLogger::inst()->err("sock_send, len = %d", length);
			return -1;
		}
		left -= ret;
		SysLogger::inst()->log("Send %d bytes, left: %d", ret, left);
		p += ret;
		ret = SOCKET_ERROR;

	} while (left > 0);

	return left;
}

int SockLib::send_all_win_packets(int flag) {
	int length = sizeof(UDPPACKET);
	char *buf = winBuf;
	int ret = 0;
	int i=0;

	for ( i = 0; i < winBufPos - 1; i++) {
		buf = winBuf + length * i;
		if (lib_sendto(sock, buf, length) < 0) {
			SysLogger::inst()->err("send_frame error: buf.len:%d, No.:%d\n", length, i);
			return -1;
		}
		PUDPPACKET pudp = (PUDPPACKET)buf;
		SysLogger::inst()->asslog("Sender: sent packet %d", pudp->seq);
		ret++;
		}
		// wait for the ACK after send the last packet of the window buffer
		if (flag) {
			if (lib_sendto(sock, buf, length) < 0) {
				SysLogger::inst()->err("send_frame error: buf.len:%d, No.:%d\n", length, i);
				return -1;
			}
		PUDPPACKET pudp = (PUDPPACKET)buf;
		SysLogger::inst()->asslog("Sender: sent packet %d", pudp->seq);
		ret++;
	} else {
		buf = winBuf + length * i;
		ret = udp_sendtoEx(sock, buf, length);
		if (ret < 0) {
			SysLogger::inst()->err("send_frame error. buf.len:%d, No.:%d\n", length, i);
			return -1;
		}
	}
	return ret;
}

int SockLib::copy_2_winbuf(const char *buf) {
	UDPPACKET udp;
	int length = sizeof(UDPPACKET);

	if (winBufPos < wSize) {
		add_udpheader(&udp, (char *)buf);
		memmove(winBuf + length * winBufPos, &udp, length);
		winBufPos++;
	}
	if (winBufPos == wSize) {
		return send_all_win_packets();
	}
	return 1;
}

int SockLib::send_file(int sock, const char *filename, int len) {
	FILE *pFile = 0;
	char buf[BUFFER_LENGTH + 1];
	int ret = -1;

	pFile = fopen(filename, "rb");
	if (pFile == NULL) {
		SysLogger::inst()->err("No such a file: %s\n", filename);
		return -1;
	}
	showFile = false;
	if (showFile) {
		printf("\n\n-------------------- File content Begin------------------------\n");
	}
	while (!feof(pFile)) {
		memset((void *)buf, 0, BUFFER_LENGTH + 1);
		//fgets(buf, BUFFER_LENGTH, pFile);
		int cnt = fread(buf, 1, BUFFER_LENGTH, pFile);

		//copy to window buffer
		if (copy_2_winbuf(buf) < 0) {
			return -1;
		}
	}

	// send packets left in the window buffer.
	ret = 0;
	do {
		if (send_all_win_packets() <= 0) {
			ret = -1;
			break;
		}
	} while (winBufPos > 0);	

	fclose(pFile);
	if (showFile) {
		printf("\n-------------------- File content End------------------------\n\n");
	}
	showFile = false;

	return ret;
}

int SockLib::recv_file(int sock, const char *filename, int len) {
	FILE *pFile = 0;
	char buf[BUFFER_LENGTH + 1];

	pFile = fopen(filename, "wb");
	if (pFile == NULL) {
		SysLogger::inst()->err("No such a file:%s\n", filename);
		return -1;
	}
	int left = len, recv_len = 0;

	while (left > 0) {
		recv_len = left > BUFFER_LENGTH ? BUFFER_LENGTH : left;
		memset((void *)buf, 0, BUFFER_LENGTH + 1);
		if (sock_recvfrom(sock, buf, recv_len)) {			// call sock_recv if TCP
			SysLogger::inst()->err("recv_file error. %d, %d, %d", recv_len, left, len);
			return -1;
		}
		fwrite(buf, 1, recv_len, pFile);
		//fputs(buf, pFile);
		left -= recv_len;
	}
	fclose(pFile);

	return 0;
}

// ---------------------------------------------
// Assignment 2 UDP

int SockLib::set_dstAddr(const char *dstHostName, int dstPort) {
	if (dstHostName == 0) {
		SysLogger::inst()->err("set_dstAddr params error");
		return -1;
	}
	// specify destination address
	memset(&dstAddr, 0, sizeof(dstAddr));
	dstAddr.sin_family = AF_INET;
	dstAddr.sin_addr.s_addr = resolve_name(dstHostName);
	dstAddr.sin_port = htons(dstPort); 

	return 0;	
}

int SockLib::udp_init(int localPort, int windowSize) {
	if (init()) {
		SysLogger::inst()->err("socket init error");
		return -1;
	}
	
	// bind receiving port
	memset(&localAddr, 0, sizeof(localAddr)); /* Zero out structure */
	localAddr.sin_family = AF_INET; /* Internet address family */
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	localAddr.sin_port = htons(localPort); /* Local port */
	if (bind(sock, (struct sockaddr *) &localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
		SysLogger::inst()->err("bind error");
		return -1;
	}

	dstAddr.sin_port = 0;

	// generate seq num
	seq = 0;
	lastSeq = -1;
	reset_statistics();	
	showFile = false;

	// 
	wSize = windowSize;
	winBuf = new char[wSize * sizeof(UDPPACKET)];
	winPos = 0;
	winBufPos = 0;
	fileSize = 0;
	
	SysLogger::inst()->out("ftp_udp starting on host: [%s:%d]. (Window Size: %d)", 
		hostname, localPort, wSize);
	return 0;
}

int SockLib::lib_recvfrom(int sock, char *buf, int length, int handshake) {
	int ret = SOCKET_ERROR, left = length;
	char *p = buf;
	fd_set readfds;
	struct timeval *tp = new timeval;
	int waitCnt = 7;		// waiting total time: waitCnt * TIMEOUT_USEC
	SOCKADDR from;
	int fromlen;
	
	tp->tv_sec = 0;
	tp->tv_usec = TIMEOUT_USEC;		// router delay 300ms * 3
	
	while (1) {
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		
		if ((ret = select(sock + 1, &readfds, NULL, NULL, tp)) == SOCKET_ERROR) {
			SysLogger::inst()->err("select error");
			return -1;
			
		} else if (ret == 0) {
			// select timeout
			if (waitCnt <= 1) {
				return 0;		// receive timeout
			}
			waitCnt--;
			
		} else if (ret > 0) {
			if (handshake == 1) {
				return 1;
			}

			// there is something to be received in the windows socket buffer
			fromlen = sizeof(from);
			ret = recvfrom(sock, p, left, 0, &from, &fromlen);
			if (ret == 0) {
				SysLogger::inst()->err("recvfrom: connection closed");
				return -1;
			} else if (ret == SOCKET_ERROR) {
				SysLogger::inst()->err("recvfrom: error");
				return -1;
			}
			
			SysLogger::inst()->log("recvfrom: %d bytes, left: %d", ret, left);
			if (dstAddr.sin_port == 0) {
				// save the destination address
				memmove((void *)&dstAddr, (void *)&from, sizeof(from));
			}
			if (ret != left) {
				return -1;
			}
			break;	// 
			
		} else {
			SysLogger::inst()->err("select unknown error");
			return -1;
		}
	}
	
	return ret;
}

int SockLib::lib_sendto(int sock, char *buf, int length) {
	int ret = SOCKET_ERROR;

	ret = sendto(sock, buf, length, 0, (SOCKADDR *)&dstAddr, sizeof(dstAddr));
	if (ret == SOCKET_ERROR) { 
		SysLogger::inst()->err("lib_sendto, len = %d", length);
		return -1;
	}
	SysLogger::inst()->log("lib_sendto: %d bytes", ret);
	if (ret != length) {
		SysLogger::inst()->err("lib_sendto, ret != length");
		return -1;
	}

	totalFramesSent++;
	return ret;
}

// param handshake: 0: wait for ack and receive it, 1: wait for ack but not receive
int SockLib::udp_sendto(int sock, char *buf, int length, int handshake) {
	int ret = SOCKET_ERROR, retryCnt = 25;
	UDPPACKET udp;

	while (retryCnt > 0) {
		// sendto destination
		if (lib_sendto(sock, buf, length) < 0) {
			return -1;
		}
		if (1) {
			PUDPPACKET pudp = (PUDPPACKET)buf;
			SysLogger::inst()->asslog("Sender: sent packet %d", pudp->seq);
		}

		// wait for ACK
		memset((void *)&udp, 0, sizeof(UDPPACKET));
		ret = lib_recvfrom(sock, (char *)&udp, sizeof(UDPPACKET));
		
		if (ret == 0) {
			// get ACK timeout, sendto again
			SysLogger::inst()->asslog("Get ACK Timeout, sendto again.");
			retryCnt--;
			reSendCnt++;
			continue;

		} else if (ret < 0) {
			return -1;
		}

		if (handshake) {
			// save handshake data
			memmove((void *)&hsData, &udp.data, sizeof(HANDSHAKE));

		} else {
			// check the sequence number
			if (udp.seq != ((seq - 1) & 0x1F)) {
				SysLogger::inst()->asslog("Get a Wrong ACK. (%d. %d)", udp.seq, seq);
				return -1;
			}
		}
		SysLogger::inst()->asslog("Sender: received ACK for packet %d", udp.seq);
		
		break; 
	} 

	return ret;
}

int SockLib::remove_packets_from_win(unsigned int recvSeq, char arrAck[]) {
	PUDPPACKET p = 0;
	int len = sizeof(UDPPACKET);

	if (winBufPos == 0) {
		SysLogger::inst()->asslog("No packet to be removed");
	}
	// remove all packets from begin whose SEQ != recvSeq;
	int i = 0, findit = 0;
	for (; i < winBufPos; i++) {
		p = (PUDPPACKET)(winBuf + len * i);
		if (p->seq == recvSeq) {
			findit = 1;
			break;
		}
	}
	if (findit) {
		// move unsent packets to the left of window buffer
		// TODO: optimize. do not need to move.
		i++;
		winBufPos -= i;
		int j=0;
		if (winBufPos > 0 && winBufPos < wSize) {
			char *p = winBuf;
			for (int j = 0; j < winBufPos; j++) {
				memmove(p + len * j, winBuf + len * (i + j), len);
				arrAck[j] = arrAck[i + j];
			}
			for (int k = j; k < SEQUENCE_NUM_MAX; k++) {
				arrAck[k] = 0;
			}
		}
// 	} else {
// 		p = (PUDPPACKET)(winBuf + len * i);
// 		if (p->seq == (recvSeq - 1) & SEQUENCE_NUM_MASK) {
// 			winBufPos = 0;
// 		}
	}
	return winBufPos;
}

// get the biggest sequence number from consecutive ACKs
int SockLib::get_max_seq(unsigned int &retSeq, PUDPPACKET pudp, char arrAck[]) {
	PUDPPACKET p = (PUDPPACKET)winBuf;
	int len = sizeof(UDPPACKET);	
	int i = 0;

	if (winBufPos == 0) {
		SysLogger::inst()->asslog("No packet in buffer.");
		return -1;
	}

	// set a tag for this seq
	for (; i < winBufPos; i++) {
		p = (PUDPPACKET)(winBuf + len * i);
		if (p->seq == pudp->seq) {
			arrAck[i] = 1;
			break;
		}
	}

	// find the biggest one
	int j = 0;
	for (; j < winBufPos; j++) {
		if (arrAck[j] == 0) {
			break;
		}
	}
	SysLogger::inst()->asslog("%d, %d,%d,%d,%d", j, arrAck[0], arrAck[1], arrAck[2], arrAck[3]);

	if (j > 0) {
		j--;
		p = (PUDPPACKET)(winBuf + len * j);
		retSeq = p->seq;

		return 1;
	}
	return 0;
}

// obsoleted
int SockLib::set_last_seq(unsigned int &retSeq, PUDPPACKET pudp) {
	PUDPPACKET p = (PUDPPACKET)winBuf;
	unsigned int maxSeq = -1, minSeq = -1;

	if (winBufPos == 0) {
		SysLogger::inst()->err("No packet in buffer.");
		return -1;
	}

	minSeq = p->seq;
	p = (PUDPPACKET)(winBuf + sizeof(UDPPACKET) * (winBufPos - 1));
	maxSeq = p->seq;

	if (seqOfLastACK == -1) {
		// the first one should be minSeq
		if (pudp->seq == minSeq) {
			seqOfLastACK = pudp->seq;
		} else {
			return 0;
		}
	} else {
		if (maxSeq > minSeq) {
			if (pudp->seq > seqOfLastACK) {
				seqOfLastACK = pudp->seq;
			}
		} else {
			// seq list like: 29,30,31,0,1,2,3
			if (pudp->seq > minSeq || pudp->seq > seqOfLastACK) {
				seqOfLastACK = pudp->seq;
			} else if (pudp->seq < minSeq || pudp->seq > seqOfLastACK) {
				seqOfLastACK = pudp->seq;
			}
		}
	} 
	SysLogger::inst()->asslog("SEQ: (%d, %d), %d, %d", minSeq, maxSeq, pudp->seq, seqOfLastACK);

	if (seqOfLastACK == maxSeq) {
		return 1;
	} else if (seqOfLastACK == minSeq) {
		return 2;
	}
	return 0;
}

int SockLib::send_special_request() {
	int length = sizeof(UDPPACKET);
	UDPPACKET udp;
	
	memset((void *)&udp, 0, length);	
	udp.seq = seq;
	udp.ackType = ACKTYPE_SACK;

	if (lib_sendto(sock, (char *)&udp, length) < 0) {
		SysLogger::inst()->err("send_special_request error: Seq:%d\n", seq);
		return -1;
	}
	return 0;
}

int SockLib::udp_sendtoEx(int sock, char *buf, int length) {
	int ret = SOCKET_ERROR, retryCnt = 5;
	boolean waitForACK = false;
	UDPPACKET udp;
	char arrACK[SEQUENCE_NUM_MAX];
	
	memset(arrACK, 0, SEQUENCE_NUM_MAX);
	while (retryCnt > 0) {
		if (!waitForACK) {
			// sendto destination
			if (lib_sendto(sock, buf, length) < 0) {
				return -1;
			}
			PUDPPACKET pudp = (PUDPPACKET)buf;
			SysLogger::inst()->asslog("Sender: sent packet %d", pudp->seq);
		}
		
		// wait for ACK
		memset((void *)&udp, 0, sizeof(UDPPACKET));
		ret = lib_recvfrom(sock, (char *)&udp, sizeof(UDPPACKET));
		
		if (ret == 0) {
			// get ACK timeout, sendto again
			SysLogger::inst()->asslog("Get ACK Timeout.");
			retryCnt--;

			// resend all packets again
			if (send_all_win_packets(1) < 0) {
				return -1;
			}
			if (retryCnt == 0) {
				// send special packet to get the sequence number the receiver is waiting for
				if (send_special_request()) {
					return -1;
				}
			}
			memset(arrACK, 0, SEQUENCE_NUM_MAX);
			continue;
			
		} else if (ret < 0) {
			return -1;
		}
		
		if (udp.ackType == ACKTYPE_ACK) {
			SysLogger::inst()->asslog("Sender: received ACK for packet %d", udp.seq);
		} else if (udp.ackType == ACKTYPE_NACK) {
			SysLogger::inst()->asslog("Sender: received NACK for packet %d", udp.seq);
		} else if (udp.ackType == ACKTYPE_SACK) {
			SysLogger::inst()->asslog("Sender: received SACK for packet %d", udp.seq);
		} else {
			// error
			SysLogger::inst()->asslog("Sender: received unknown ACK type: %d, seq: %d, last: %d", 
				udp.ackType, udp.seq, lastSeq);
			// return -1;			// it might be a resent packet, send ack and discards it.
			if (udp.seq == ((lastSeq + 1) & SEQUENCE_NUM_MASK)) {
				send_ack(udp.seq, ACKTYPE_ACK);
				waitForACK = true;
			}
		}

		// if it is a NACK
		if (udp.ackType == ACKTYPE_NACK || udp.ackType == ACKTYPE_SACK) {
			// remove received packets from window buffer
			UDPPACKET tmp;

			tmp.seq = (udp.seq - 1) & SEQUENCE_NUM_MASK;
			int packets_left = remove_packets_from_win(tmp.seq, arrACK);
			SysLogger::inst()->asslog("remove_packets: %d, %d", packets_left, tmp.seq);
			
			// read file again and send all packets
			return packets_left;
			
		} else if (udp.ackType == ACKTYPE_ACK) {
			// check the sequence number
			unsigned int biggestACK = -1;
			int tmp = get_max_seq(biggestACK, &udp, arrACK);
	
			if (tmp == -1) {
				return -1;
			} 
			if (tmp == 1) {
				// remove packets from window buffer
				int packets_left = remove_packets_from_win(biggestACK, arrACK);

				SysLogger::inst()->asslog("remove_packets: %d, %d", packets_left, biggestACK);
				if (packets_left == 0) {
					return 0;
				}
			}
			
			// do not find the ack for the last packet. continue waiting...
			waitForACK = true;
		}
	} 
	
	return -1;
}

int SockLib::add_udpheader(PUDPPACKET pudp, char *buf) {
	memset((void *)pudp, 0, sizeof(UDPPACKET));

	pudp->seq = seq++;
	pudp->ackType = ACKTYPE_REQUEST;
	if (buf) {
		memmove(pudp->data, buf, BUFFER_LENGTH);
	}
	return 0;
}

int SockLib::sock_sendto(int sock, char *buf, int length, int handshake) {
	int ret = SOCKET_ERROR, left = length;
	UDPPACKET udp;
	char *p = buf;

	// divide the tcp packet into small udp frames
	while (left > 0) {
		// add udp header and copy the data
		if (add_udpheader(&udp, p)) {
			return -1;
		}
		if (handshake) {
			udp.ackType = ACKTYPE_HANDSHAKE;
		}
		if (udp_sendto(sock, (char *)&udp, sizeof(UDPPACKET), handshake) <= 0) {
			return -1;
		}

		left -= BUFFER_LENGTH;
		p = buf + BUFFER_LENGTH;
		sendCnt++;

		if (showFile) {
			printf("%s\n", udp.data);
		}
	}
	if (left < 0) {
		left = 0;		// to be compatible with TCP
	}

	return left;
}

int SockLib::sock_sendtoEx(int sock, char *buf, int length, int flag) {
	//copy to window buffer
	if (copy_2_winbuf(buf) < 0) {
		return -1;
	}
	
	// send packets left in the window buffer.
	int ret = -1;
	if (flag) {
		//ret = send_all_win_packets(flag);
		return 0;
	} else {
		// wait for response
		do {
			ret = send_all_win_packets(flag);
			if (ret <= 0) {
				break;
			}
		} while (winBufPos > 0);
	}

	if (ret >= 0) {
		return 0;
	}
	return -1;
}

// send an ACK to the sender
int SockLib::send_ack(unsigned int seq, int type) {
	UDPPACKET udp;

	memset((void *)&udp, 0, sizeof(UDPPACKET));	
	udp.seq = seq;
	udp.ackType = type;
	lib_sendto(sock, (char *)&udp, sizeof(UDPPACKET));

	if (udp.ackType == ACKTYPE_ACK) {
		SysLogger::inst()->asslog("Receiver: sent an ACK for packet %d", udp.seq);
	} else if (udp.ackType == ACKTYPE_NACK) {
		SysLogger::inst()->asslog("Receiver: sent an NACK for packet %d", udp.seq);
	} else if (udp.ackType == ACKTYPE_SACK) {
		SysLogger::inst()->asslog("Receiver: sent an SACK for packet %d", udp.seq);
	}
	return 0;
}

// used by receiver to check if the frame is the same as last one
int SockLib::chk_seq(unsigned int frame_seq) {
	if (lastSeq == -1) {
		lastSeq = seq;
		return 0;
	}

	if (lastSeq == frame_seq) {
		return -1;			// duplicated frame
	}

	lastSeq = frame_seq;
	return 0;
}
int SockLib::chk_seqEx(PUDPPACKET pudp) {
	if (lastSeq == -1) {
		lastSeq = (pudp->seq - 1) & SEQUENCE_NUM_MASK;
	}

	if (pudp->ackType == ACKTYPE_REQUEST) {
		if (((lastSeq + 1) & SEQUENCE_NUM_MASK) != pudp->seq) {
			send_ack(lastSeq + 1, ACKTYPE_NACK);
		} else {
			send_ack(lastSeq + 1, ACKTYPE_ACK);
			lastSeq = (lastSeq + 1) & SEQUENCE_NUM_MASK;
			return 0;
		}

	} else if (pudp->ackType == ACKTYPE_SACK) {
		// if it is a special packet
		send_ack(lastSeq + 1, ACKTYPE_SACK);
	}
	return -1;
}

int SockLib::sock_recvfrom(int sock, char *buf, int length, int handshake) {
	int ret = SOCKET_ERROR, left = length;
	char *p = buf;
	UDPPACKET udp;

	do {
		memset((void *)&udp, 0, sizeof(UDPPACKET));		
		ret = lib_recvfrom(sock, (char *)&udp, sizeof(UDPPACKET));
		if (ret == 0) {
			continue;
		} else if (ret < 0) {
			return -1;
		} 

		SysLogger::inst()->asslog("Receiver: received packet %d, %d", udp.seq, udp.ackType);

		HANDSHAKE tmp;
		tmp.clientSeq = 0;
		tmp.serverSeq = 0;
		memmove((void *)&tmp, &udp.data, sizeof(HANDSHAKE));

		if (handshake) {
			// send response to handshake request
			UDPPACKET resp;
			
			memset((void *)&resp, 0, sizeof(UDPPACKET));
			resp.seq = udp.seq;
			if (tmp.serverSeq == 0) {
				SysLogger::inst()->out("Received a Handshake Request (%d, %d)", tmp.clientSeq, tmp.serverSeq);
				hsData.clientSeq = tmp.clientSeq;
			} else {
				SysLogger::inst()->out("Received a Handshake Response (%d, %d)\n", tmp.clientSeq, tmp.serverSeq);
				if (hsData.clientSeq != tmp.clientSeq || hsData.serverSeq != tmp.serverSeq) {
					return -1;
				}
			}
			memmove((void *)&resp.data, &hsData, sizeof(HANDSHAKE));
			lib_sendto(sock, (char *)&resp, sizeof(UDPPACKET));
			if (tmp.serverSeq == 0) {
				SysLogger::inst()->out("Sent a Handshake Response (%d, %d)", hsData.clientSeq, hsData.serverSeq);
			} else {
				//SysLogger::inst()->out("Sent Handshake last ACK (%d, %d)", hsData.clientSeq, hsData.serverSeq);
			}

			if (chk_seq(udp.seq)) {
				SysLogger::inst()->asslog("Receiver: received same packet %d", udp.seq);
				continue;
			}	
		} else {
			// check if it is still a handshake frame, then discards it until receive a normal frame
			//if (hsData.clientSeq == tmp.clientSeq && hsData.serverSeq == tmp.serverSeq) {
			if (udp.ackType == ACKTYPE_HANDSHAKE) {
				// send ACK 
				UDPPACKET resp;
				resp.seq = udp.seq;
				memset((void *)&resp, 0, sizeof(UDPPACKET));
				memmove((void *)&resp.data, &hsData, sizeof(HANDSHAKE));
				lib_sendto(sock, (char *)&resp, sizeof(UDPPACKET));
				SysLogger::inst()->out("--==resp ack: %d", udp.seq);
				continue;
			}

			// send ACK
			//send_ack(udp.seq);			
		}

		// check sequence number
		if (!handshake) {
			if (chk_seqEx(&udp)) {
				SysLogger::inst()->asslog("Receiver: received disorder packet. LastSeq: %d, curSeq: %d", 
					lastSeq, udp.seq);
				continue;
			}
		}

		// copy the data from udp frame
		memmove(p, udp.data, (left < BUFFER_LENGTH) ? left : BUFFER_LENGTH );
		left -= BUFFER_LENGTH;
		SysLogger::inst()->log("sock_recvfrom %d bytes, left: %d", ret, left);
		p += BUFFER_LENGTH;
		ret = SOCKET_ERROR;
		recvCnt++;
	} while (left > 0);
	
	if (left < 0) {
		left = 0;		// to be compatible with TCP
	}

	return left;
}


// deal with ACK of last packet
int SockLib::srv_wait4cnn(int sock, int sec) {
	int ret = SOCKET_ERROR;
	UDPPACKET udp;
	
	do {
		memset((void *)&udp, 0, sizeof(UDPPACKET));		
		ret = lib_recvfrom(sock, (char *)&udp, sizeof(UDPPACKET));
		if (ret == 0) {
			continue;
		} else if (ret < 0) {
			return -1;
		}
		
		SysLogger::inst()->asslog("Receiver: received packet %d", udp.seq);		
		
		// send ACK
		send_ack(udp.seq, ACKTYPE_ACK);			
		
		// check sequence number
// 		if (chk_seq(udp.seq)) {
// 			SysLogger::inst()->asslog("Receiver: received same packet %d", udp.seq);
// 			continue;
// 		}	
		
	} while (sec-- > 0);
	
	return 0;
}

void SockLib::reset_statistics(bool seq) {
	reSendCnt = 0;
	recvCnt = 0;
	sendCnt = 0;
	totalFramesSent = 0;
	fileSize = 0;
	if (seq) {
		lastSeq = -1;
	}
}
void SockLib::show_statistics(bool ifSend) {
// 	if (ifSend) {
// 		SysLogger::inst()->out("Sender: number of effective bytes sent: %d (%d * %d)", 
// 			sendCnt * sizeof(UDPPACKET), sendCnt, sizeof(UDPPACKET));
// 		SysLogger::inst()->out("Sender: number of packets sent: %d", sendCnt + reSendCnt);
// 		SysLogger::inst()->out("Sender: number of bytes sent: %d (%d * %d)\n", 
// 			(sendCnt + reSendCnt) * sizeof(UDPPACKET), (sendCnt + reSendCnt), sizeof(UDPPACKET));
// 	} else {
// 		SysLogger::inst()->out("Receiver: number of bytes received: %d (%d * %d)\n", 
// 			recvCnt * sizeof(UDPPACKET), recvCnt, sizeof(UDPPACKET));
// 	}
	if (ifSend) {
		SysLogger::inst()->out("Sender: number of effective bytes sent: %d (%d * %d)", 
			fileSize, fileSize / sizeof(UDPPACKET) + 1, sizeof(UDPPACKET));
		SysLogger::inst()->out("Sender: number of packets sent: %d", totalFramesSent);
		SysLogger::inst()->out("Sender: number of bytes sent: %d (%d * %d)\n", 
			(totalFramesSent) * sizeof(UDPPACKET), (totalFramesSent), sizeof(UDPPACKET));
	} else {
		SysLogger::inst()->out("Receiver: number of bytes received: %d (%d * %d)\n", 
			recvCnt * sizeof(UDPPACKET), recvCnt, sizeof(UDPPACKET));
	}
	reset_statistics();
}

void SockLib::setWindowsSize(int size) {
	wSize = size;
}

