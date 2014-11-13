/*
   COMP6461 Assignment2

   Chenglong Zhang (ID: 6842666)
   Liu Sun         (ID: 6758878)

 * This file is created by Chenglong Zhang
 *
 * $Author$
 * $Date$
 * $Rev$
 * $HeadURL$
 *
 */

/*From COEN320:Real-Time System and COEN421: Embedded System Design*/
#ifndef THREAD_HPP
#define THREAD_HPP

#include <stdio.h>
#include <stdlib.h>
#include <process.h>

#define	STKSIZE	 16536
class Thread {
public:

	Thread() {
	}
	virtual ~Thread() {
	}

	static void * pthread_callback(void * ptrThis);

	virtual void run() =0;
	void start();
};
#endif
