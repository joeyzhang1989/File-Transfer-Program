/*
 * COMP6461 Assignment3
 *
 * This file is created by Yuan Tao (ewan.msn@gmail.com)
 * Licensed under GNU GPL v3
 *
 * $Author$
 * $Date$
 * $Rev$
 * $HeadURL$
 *
 */

#ifndef __SYSLOGGER_H__
#define __SYSLOGGER_H__


#define TRACE	2		// 0: no trace, 1: all trace, 2: trace requried by assignment


class SysLogger {
private:
	static SysLogger *pInst;		// TODO: delete
	static FILE *pLogFile;
	SysLogger();
	~SysLogger();

public:
	static SysLogger *inst();
	
	int set(char *filename);

	void err(char *fmt, ...);
	void log(char *fmt, ...);
	void asslog(char *fmt, ...);		// trace requried by assignment
	void out(char *fmt, ...);

	void wellcome();
};


#endif


