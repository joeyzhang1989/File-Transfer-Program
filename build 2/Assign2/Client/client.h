/*
 * COMP6461 Assignment2
 *
 * This file is created by Chenglong Zhang
 *
 * $Author$
 * $Date$
 * $Rev$
 * $HeadURL$
 *
 */
#ifndef __CLIENT_H__
#define __CLIENT_H__

const char *FILE_DIR_ROOT = "../client_files_root/";

class SockClient: public SockLib {

public:
	SockClient(){};
	~SockClient(){};
	int start(const char *filename, const char *opname);
	int handshake();
};


#endif


