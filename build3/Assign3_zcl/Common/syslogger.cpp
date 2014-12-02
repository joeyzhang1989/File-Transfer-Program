
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

#include "syslogger.h"

SysLogger * SysLogger::pInst = NULL;
FILE * SysLogger::pLogFile = NULL;

SysLogger::SysLogger() {
}
SysLogger::~SysLogger() {
	fclose(pLogFile);
}

SysLogger *SysLogger::inst() {
	if (pInst == NULL) {
		pInst = new SysLogger();
	}
	return pInst;
}
	
// set a log file.
int SysLogger::set(char *filename) {
	if (filename == NULL) {
		return -1;
	}
	pLogFile = fopen (filename , "w");
	if (pLogFile == NULL) {
		printf("Failed to fopen\n");
		return -1;
	}
	return 0;
}

void SysLogger::err(char *fmt, ...) {
	if (pLogFile == NULL || !TRACE) {
		return;
	}
	va_list args;
	va_start(args, fmt);
	vfprintf(pLogFile, fmt, args);
	fprintf(pLogFile, "\n");
	vfprintf(stdout, fmt, args);
	fprintf(stdout, "\n");
	va_end(args);

	fprintf(pLogFile, "WSAGetLastError:%d\n", WSAGetLastError());
	//fprintf(stdout, "WSAGetLastError:%d\n", WSAGetLastError());
	fflush(pLogFile);
}

void SysLogger::log(char *fmt, ...) {
	if (pLogFile == NULL || !(TRACE == 1)) {
		return;
	}
	va_list args;
	va_start(args, fmt);
	vfprintf(pLogFile, fmt, args);
	fprintf(pLogFile, "\n");
	va_end(args);
	fflush(pLogFile);
}

void SysLogger::asslog(char *fmt, ...) {
	if (pLogFile == NULL || !TRACE) {
		return;
	}
	va_list args;
	va_start(args, fmt);
	vfprintf(pLogFile, fmt, args);
	fprintf(pLogFile, "\n");
	va_end(args);
	fflush(pLogFile);
}

// messages showing to the users
void SysLogger::out(char *fmt, ...) {
	if (pLogFile == NULL || !TRACE) {
		return;
	}
	va_list args;
	va_start(args, fmt);
	vfprintf(pLogFile, fmt, args);
	fprintf(pLogFile, "\n");
	vfprintf(stdout, fmt, args);
	fprintf(stdout, "\n");
	va_end(args);
	fflush(pLogFile);
}

void SysLogger::wellcome() {
	out("Welcome to COMP6461 assignment 3.");
	out("Developed by Chenglong Zhang and Liu Sun.\n");
	out("If any error of Client or Server happens, please check the logs first, which are placed under $ThisProgram\\logs\\. ");
	out("");
}



