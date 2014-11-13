//    SERVER TCP PROGRAM
// revised and tidied up by
// J.W. Atwood
// 1999 June 30
// There is still some leftover trash in this code.

/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma once
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <ws2tcpip.h>
#include <process.h>
#include <winsock.h>
#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tchar.h>
#include <direct.h>

using namespace std;

//port data types

#define REQUEST_PORT 5001
#define	STKSIZE	 16536

// fill with server info, IP, port
union {struct sockaddr generic;
	struct sockaddr_in ca_in;}ca;

	char szbuffer[128];
	int calen=sizeof(ca); 

	//buffer data types

	char *buffer;
	int ibufferlen;
	int ibytesrecv;
	SOCKET s1;
	SOCKET s;
	int ibytessent;

	//host data types
	char localhost[11];

	HOSTENT *hp;

	//wait variables
	int nsa1;
	int r,infds=1, outfds=0;
	struct timeval timeout;
	const struct timeval *tp=&timeout;

	fd_set readfds;


#define HOSTNAME_LENGTH 20
#define RESP_LENGTH 40
#define FILENAME_LENGTH 20
#define BUFFER_LENGTH 128 
#define TRACE 0


	int num = 0;

typedef struct
{
	char filename[260];
	long filelenth;
}Fileinfo;

class TcpServer
{
	SOCKADDR_IN sa;      // filled by bind
	int port;

	public:
		TcpServer();
		~TcpServer();
		void TcpServer::start();
};

class Thread{
	public:
		Thread()
		{}
		virtual ~Thread()
		{}
		static void * pthread_callback (void * ptrThis);
		virtual void run () =0 ;
		void  start();
};


void * Thread::pthread_callback (void * ptrThis) 
{
    if ( ptrThis == NULL )
	     return NULL ;	
    Thread  * ptr_this =(Thread *)(ptrThis) ;
    ptr_this->run();
    return NULL;
} 

void Thread::start () 
{
	int threadresul;
    if(( threadresul = _beginthread((void (*)(void *))Thread::pthread_callback,STKSIZE,this ))<0)
	{
		printf("_beginthread error\n");
		exit(-1);
	}
	else
	{
		printf("Thread:%d creat ok\n",num);
		num++;
	}
	
}

	class TcpThread :public Thread
{         
	int cs;						 /* Socket descriptor */
	struct sockaddr_in ServAddr; /* server socket address */
	unsigned short ServPort;     /* server port */	
	WSADATA wsadata;
public:
	TcpThread(int clientsocket):cs(clientsocket)
	{}
	virtual void run();
	void listFiles(SOCKET ss);
	void PutFileToClient(SOCKET ps);
	void GetFileFromClient(SOCKET gs);

};
	
	void TcpThread::run()
	{
				memset(szbuffer,0,sizeof(szbuffer));
				//Fill in szbuffer from accepted request.
				if((ibytesrecv = recv(s1,szbuffer,128,0)) == SOCKET_ERROR)
					throw "Receive error in server program\n";
				//Print reciept of successful message. 
				cout << "This is message from client: " << szbuffer << endl;		
				if(strcmp(szbuffer,"LIST\r")==0)
					listFiles(s1);

				if(strcmp(szbuffer,"GET\r")==0)
				{
					if((ibytessent = send(s1,szbuffer,128,0))==SOCKET_ERROR)
						throw "error in send in server program\n";
					else cout << "Operation:" << szbuffer << endl;  
					PutFileToClient(s1);
				}

				if(strcmp(szbuffer,"PUT\r")==0)
				{
					if((ibytessent = send(s1,szbuffer,128,0))==SOCKET_ERROR)
						throw "error in send in server program\n";
					else cout << "Operation:" << szbuffer << endl;  
					GetFileFromClient(s1);
				}
	}

	void TcpServer::start()
	{
			while(1)
			{
				FD_SET(s,&readfds);  //always check the listener
				if(!(outfds=select(infds,&readfds,NULL,NULL,tp))) {}
				else if (outfds == SOCKET_ERROR) throw "failure in Select";
				else if (FD_ISSET(s,&readfds))  cout << "got a connection request" << endl; 
				//Found a connection request, try to accept. 
				if((s1=accept(s,&ca.generic,&calen))==INVALID_SOCKET)
					throw "Couldn't accept connection\n";
				//Connection request accepted.
				cout<<"accepted connection from "<<inet_ntoa(ca.ca_in.sin_addr)<<":"
					<<hex<<htons(ca.ca_in.sin_port)<<endl;
				TcpThread *pt=new TcpThread(s1);
				pt->start();
			}
	}

	TcpServer::TcpServer()
	{
		port = REQUEST_PORT;
		WSADATA wsadata;
    		 
			if (WSAStartup(0x0202,&wsadata)!=0){  
				cout<<"Error in starting WSAStartup()\n";
			}else{
				buffer="WSAStartup was suuccessful\n";   

				/* display the wsadata structure */
				cout<< endl
					<< "wsadata.wVersion "       << wsadata.wVersion       << endl
					<< "wsadata.wHighVersion "   << wsadata.wHighVersion   << endl
					<< "wsadata.szDescription "  << wsadata.szDescription  << endl
					<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
					<< "wsadata.iMaxSockets "    << wsadata.iMaxSockets    << endl
					<< "wsadata.iMaxUdpDg "      << wsadata.iMaxUdpDg      << endl;
			}  

			//Display info of local host

			gethostname(localhost,100);
			cout<<"hostname: "<<localhost<< endl;

			if((hp=gethostbyname(localhost)) == NULL) {
				cout << "gethostbyname() cannot get local host info?"
					<< WSAGetLastError() << endl; 
				exit(1);
			}

			//Create the server socket
			if((s = socket(AF_INET,SOCK_STREAM,0))==INVALID_SOCKET) 
				throw "can't initialize socket";
			// For UDP protocol replace SOCK_STREAM with SOCK_DGRAM 


			//Fill-in Server Port and Address info.
			sa.sin_family = AF_INET;
			sa.sin_port = htons(port);
			sa.sin_addr.s_addr = htonl(INADDR_ANY);


			//Bind the server port

			if(bind(s,(LPSOCKADDR)&sa,sizeof(sa))==SOCKET_ERROR)
				throw "can't bind the socket";
			cout << "Bind was successful" << endl;

			//Successfull bind, now listen for client requests.

			if(listen(s,10) == SOCKET_ERROR)
				throw "couldn't  set up listen on socket";
			else cout << "Listen was successful" << endl;

			FD_ZERO(&readfds);
	}

	TcpServer::~TcpServer()
	{
	WSACleanup();
	}

	
	void TcpThread::listFiles(SOCKET ss1)

 {
	struct _finddata_t serverfile;
    long serverfileHandle;
	char serverdirbuffer[256];
	getcwd(serverdirbuffer,256);

    //string curPath";
	string curPath = strcat( serverdirbuffer,"\\*.*");

    if ((serverfileHandle = _findfirst(curPath.c_str(), &serverfile)) == -1) 
    {
        return;
    }    
    do {
            if (_A_ARCH == serverfile.attrib) 
            {				
				if(send(ss1,serverfile.name,sizeof(serverfile.name),0) != sizeof(serverfile.name))
					printf("error in send in server program\n");
        }
    } while (!(_findnext(serverfileHandle, &serverfile)));
    _findclose(serverfileHandle);
				char servbuffer[260];
				servbuffer[0] = '@';
				if(send(ss1,servbuffer,sizeof(servbuffer),0) != sizeof(servbuffer))
					printf("error in send in server program\n");				
}

	int main(void){

				TcpServer tserver;
				tserver.start();

		//close Client socket
		closesocket(s1);		

		//close server socket
		closesocket(s);

		/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
		WSACleanup();
		return 0;
	}




