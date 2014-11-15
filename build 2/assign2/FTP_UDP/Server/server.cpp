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
#include <sstream>
#include<time.h>

using namespace std;

//port data types

#define REQUEST_PORT 5001
#define	STKSIZE	 16536
#define BUFFER_LENGTH 260 
#define USEC 300000
#define SECT 0 

// fill with server info, IP, port
union {struct sockaddr generic;
	struct sockaddr_in ca_in;}ca;

	char szbuffer[BUFFER_LENGTH];
	int calen=sizeof(ca); 
	char *buffer;
	int ibufferlen;
	SOCKET s1;
	int ibytessent;
	char localhost[11];
	HOSTENT *hp;
	int r,infds=1, outfds=0;
	struct timeval timeout;
	const struct timeval *tp=&timeout;
	fd_set readfds;
	SOCKET s;
	SOCKADDR_IN sa;      // filled by bind
	SOCKADDR_IN sa1;     // fill with server info, IP, port

	//buffer data types
	char fileSzbuffer[512];

	int client_length;
	string cli_file;
	string cli_dir;
	string cli_size;
	//host data types
	//wait variables
	int nsa1;

	int client_Start_Seq;
	int server_Start_Seq;
	int prev_Recv_Seq;
	int current_Recv_Seq;
	int Send_Seq;
	int next_Recv_Seq;
	//others
	HANDLE test;
	DWORD dwtest;
	stringstream out;
//	int num = 0; test multithread

typedef struct
{
	char filename[260];
	long filelenth;
}Fileinfo;

typedef struct
{
	int seqNo;
	char data[BUFFER_LENGTH];
}Udppacket;


	class TcpServer
{         
	int cs;						 /* Socket descriptor */
	struct sockaddr_in sa; /* server socket address */
	struct sockaddr_in sa_in;
	unsigned short ServPort;     /* server port */	
	WSADATA wsadata;

	int client_Start_Seq;
	int server_Start_Seq;
	int prev_Recv_Seq;
	int next_Rec_Seq;
	int current_Recv_Seq;
	int Send_Seq;
	int totalsendnumberpackets;
	int totalsendpacket;
	int totalrecvnumberpackets;
	int totalrecvpacket;
	int outfds;
	Udppacket sendmsg;
	Udppacket recvmsg;
	int ibytesrecv;
	int ibytesput;
	string ack;
	struct timeval timeout;

public:
	TcpServer();
	virtual void run();
	void listFiles(SOCKET ss);
	void PutFileToClient(SOCKET ps);
	void GetFileFromClient(SOCKET gs);
	void delFiles(SOCKET ds);
	bool threeWayHandShake();
	Udppacket udpsend(SOCKET sock,char *sbuffer,int buflength, int sendflag);
	Udppacket udprecv(SOCKET sock,char *sbuffer,int buflength, int sendflag);
	~TcpServer();
};
	
	void TcpServer::run()
	{
		Udppacket udpbuf;
		int msglength = sizeof(udpbuf);
		Send_Seq = 0;
		next_Rec_Seq =0;

//		int port = REQUEST_PORT;
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
			if((s1 = socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET) 
				throw "can't initialize socket";
			// For UDP protocol replace SOCK_STREAM with SOCK_DGRAM 


			//Fill-in Server Port and Address info.
			sa.sin_family = AF_INET;
			sa.sin_port = htons(REQUEST_PORT);
			sa.sin_addr.s_addr = htonl(INADDR_ANY);


			//Bind the server port

			if(bind(s1,(LPSOCKADDR)&sa,sizeof(sa))==SOCKET_ERROR)
				throw "can't bind the socket";
			cout << "Bind was successful" << endl;

			//Successfull bind, now listen for client requests.

			FD_ZERO(&readfds);

		while(1)
			{
				if(!(outfds=select(infds,&readfds,NULL,NULL,tp))) {}
				else if (outfds == SOCKET_ERROR) ;
				else if (FD_ISSET(s1,&readfds))  cout << "got a connection request" << endl; 
				//Found a connection request, try to accept. 
//				if((s1=accept(s,&ca.generic,&calen))==INVALID_SOCKET)
//					throw "Couldn't accept connection\n";
				//Connection request accepted.
//				cout<<"accepted connection from "<<inet_ntoa(ca.ca_in.sin_addr)<<":"
//					<<hex<<htons(ca.ca_in.sin_port)<<endl;

		memset(udpbuf.data,0,sizeof(udpbuf.data));
		memset(szbuffer,0,sizeof(szbuffer));
				//Fill in szbuffer from accepted request.
//				if((ibytesrecv = recv(s1,szbuffer,128,0)) == SOCKET_ERROR)
//					throw "Receive error in server program\n";
				//Print reciept of successful message. 
			udpbuf = udprecv(s1,szbuffer,128,0);

			memcpy(szbuffer,recvmsg.data,BUFFER_LENGTH);
			cout << "This is message from client: " << recvmsg.data << endl;		
				if(strcmp(szbuffer,"LIST\r")==0)
					listFiles(s1);

				if(strcmp(szbuffer,"GET\r")==0)
				{
					udpbuf = udpsend(s1,szbuffer,128,0);
					cout << "Operation:" << udpbuf.data << endl;  
					PutFileToClient(s1);
				}

				if(strcmp(szbuffer,"PUT\r")==0)
				{
					udpbuf = udpsend(s1,szbuffer,128,0);
					cout << "Operation:" << udpbuf.data << endl;  
					GetFileFromClient(s1);
				}
				if(strcmp(szbuffer,"DEL\r")==0)
				{
//					if((ibytessent = send(s1,szbuffer,128,0))==SOCKET_ERROR)
//						throw "error in send in server program\n";
					udpbuf = udpsend(s1,szbuffer,128,0);
					cout << "Operation:" << udpbuf.data << endl;  
					delFiles(s1);
				}
		}
	}

	Udppacket TcpServer::udpsend(SOCKET sock,char *sbuffer,int buflength, int sendflag)
{
	char recvbuffer[BUFFER_LENGTH];
	char sendbuffer[BUFFER_LENGTH];
	int msglength = sizeof(struct sockaddr_in);

	memset(sendmsg.data,0,BUFFER_LENGTH);
	sendmsg.seqNo=Send_Seq;
	memcpy(sendmsg.data,sbuffer,BUFFER_LENGTH);

//	int ibytesput = send(sock,msg.data,128,0);
	if(sendflag == 0)
		ibytesput = sendto(sock,(char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, BUFFER_LENGTH);
	else
	{
		string strbuf(sbuffer);
		strcpy(sendmsg.data,(strbuf.substr(0,sendflag)).c_str());
		ibytesput = sendto(sock,(char*)&sendmsg, sendflag, 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));
	}
//	cout<<"send:"<<sendmsg.data<<endl;
	if (ibytesput == -1)
				fprintf(stderr, "Error transmitting data.\n");
/*			if(TRACE)
			{
				fout<<"Sender: sent packet "<<Send_Seq<<endl;
			}
*/
			totalsendpacket++;		

			//wait for ack.
			do{
				fd_set readfds; //fd_set is a type
				FD_ZERO(&readfds); //initialize
				FD_SET(sock, &readfds); //put the socket in the set
				timeout.tv_sec = SECT;
				timeout.tv_usec = USEC;
				if(!(outfds = select (1 , &readfds, NULL, NULL, & timeout)))
				{//if timed out, resent
					if(sendflag == 0)
						ibytesput = sendto(sock,(char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, BUFFER_LENGTH);
					else
						{
							string strbuf(sbuffer);
							strcpy(sendmsg.data,(strbuf.substr(0,sendflag)).c_str());
							ibytesput = sendto(sock,(char*)&sendmsg, sendflag, 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));
						}
					if (ibytesput == -1)
								fprintf(stderr, "Error transmitting data.\n");
//					cout<<"resend:"<<sendmsg.data<<endl;
/*					if(TRACE)
					{
						fout<<"Sender: resent packet "<<Send_Seq<<endl;
					}
*/
					totalsendpacket++;
				}
				if (outfds == 1) 
				{//if not timed out, receive ack, store ack seq number
					memset(recvmsg.data,0,BUFFER_LENGTH);
					ibytesrecv = recvfrom(sock, (char*)&recvmsg, sizeof(recvmsg), 0, (struct sockaddr *)&sa_in, &msglength);
/*					if(TRACE)
					{
						fout<<"Sender: received ACK for packet "<<Send_Seq<<endl;
					}
*/
//					cout<<"recieve ack:"<<recvmsg.data<<endl;
					totalsendnumberpackets++;
				}
			}while(outfds!=1);
			Send_Seq= abs(Send_Seq-1);

	return sendmsg;
}



Udppacket TcpServer::udprecv(SOCKET sock,char *sbuffer,int buflength, int sendflag)
{
//	char recvbuffer[BUFFER_LENGTH];
	char sendbuffer[BUFFER_LENGTH];
	int msglength = sizeof(struct sockaddr_in);
	totalrecvnumberpackets = 0;
	totalrecvpacket = 0;

do{
		int q = sizeof(recvmsg);
		int p = msglength;
		memset(recvmsg.data,0,BUFFER_LENGTH);
        ibytesrecv = recvfrom(sock, (char*)&recvmsg, sizeof(recvmsg), 0, (struct sockaddr *)&sa_in, &msglength);
//		cout<<"recieve:"<<recvmsg.data<<endl;
		totalrecvnumberpackets++;
        int seq=recvmsg.seqNo;        
        if(ibytesrecv>0)
		{					
            stringstream out;
            current_Recv_Seq=recvmsg.seqNo;
            out << current_Recv_Seq;
            ack="serverACK"+out.str();

            if(current_Recv_Seq==next_Rec_Seq)
			{
				memset(sendmsg.data,0,BUFFER_LENGTH);
				memcpy(sendmsg.data,ack.c_str(),BUFFER_LENGTH);
				totalrecvpacket++;
                if (sendto(sock, (char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, msglength) == -1)
                    fprintf(stderr, "Error transmitting data.\n");	
//				cout<<"send ack:"<<sendmsg.data<<endl;
            }
            // else if seq number=previous seq number
            else 
			{               
				memset(sendmsg.data,0,BUFFER_LENGTH);
				memcpy(sendmsg.data,ack.c_str(),BUFFER_LENGTH);

                if (sendto(sock, (char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, msglength) == -1)
                    fprintf(stderr, "Error transmitting data.\n");
            }

        }
    }while(current_Recv_Seq==prev_Recv_Seq);			
    prev_Recv_Seq=current_Recv_Seq;
    next_Rec_Seq= abs(next_Rec_Seq-1);     

	return recvmsg;
}


	TcpServer::TcpServer()
	{
		
	}

	TcpServer::~TcpServer()
	{
	WSACleanup();
	}

	void TcpServer::GetFileFromClient(SOCKET gs1)
	{
		char getclientfilename[260];
		Fileinfo clientputfilename;
		char serverrecvbuffer[BUFFER_LENGTH];
		struct _stat getfilestate;
		Udppacket udpbuf;
		string con_msg,cli_dir;

		memset(getclientfilename,0,sizeof(getclientfilename));
		//receive the response
//		if(recv(gs1,(char *)&clientputfilename,sizeof(clientputfilename),0)!=sizeof(clientputfilename))
//			printf("recv response error,exit");

		udpbuf = udprecv(gs1,(char *)&clientputfilename,sizeof(clientputfilename),0);

		for (int i = 0; udpbuf.data[i] != 0; i++)
		con_msg += udpbuf.data[i];
		int pos = con_msg.find_first_of(',');
		cli_dir = con_msg.substr(0, pos);
		con_msg = con_msg.substr(pos + 1);

		
		strcpy(clientputfilename.filename,cli_dir.c_str());
		clientputfilename.filelenth = atoi(con_msg.c_str());

		//cast it to the response structure
		printf("File name: %s\n",clientputfilename.filename);
		printf("Response:file size %ld\n",clientputfilename.filelenth);
		
		//check if file exist on server, compare the names of filename and the response in respp structure
		if( _stat(clientputfilename.filename,&getfilestate)==0)
			printf("Over write file on Server:%s\n",clientputfilename.filename);
			// get the filesize
			int fsize = clientputfilename.filelenth;
	
			// start getting the file
			int nrbyte =0; // number of received bytes
			FILE *fp;
			fp = fopen (clientputfilename.filename,"wb");
	
			cout << "Size of file to be received " << fsize << endl;

			while (fsize > 0)
			{
				if (fsize > BUFFER_LENGTH)
				{
//					nrbyte = recv(gs1,serverrecvbuffer,BUFFER_LENGTH,0);
					udpbuf = udprecv(gs1,(char *)&clientputfilename,sizeof(clientputfilename),0);
					memcpy(serverrecvbuffer,udpbuf.data,sizeof(udpbuf.data));
					nrbyte = sizeof(udpbuf.data);
					fwrite(serverrecvbuffer,1,nrbyte,fp);
				}
				else
				{
//					nrbyte = recv(gs1,serverrecvbuffer,fsize,0);
					udpbuf = udprecv(gs1,(char *)&clientputfilename,sizeof(clientputfilename),0);
					memcpy(serverrecvbuffer,udpbuf.data,sizeof(udpbuf.data));
					nrbyte = fsize;
					fwrite(serverrecvbuffer,1,nrbyte,fp);
				}
	
				fsize = fsize - nrbyte;
	
			}
			fclose(fp);
			cout << "Download Finished" << endl;
			closesocket(gs1);
	}
	
bool TcpServer::threeWayHandShake(){
	bool success = true; string prevMsg;
	//get details from buffer
	string clientNum;
	string request;
	string randomNum;
	for (int i = 0; szbuffer[i] != 0; i++)
		request += szbuffer[i];
	prevMsg = request;
	clientNum = request;

	//send ack with server random number
	srand(time (0));
	int random_integer = rand() % 256;
	out.str("");
	out << random_integer;
	randomNum = out.str();
	string ss;
	ss = randomNum;

	memset(szbuffer, 0, 512);
	memcpy(szbuffer, ss.c_str(), 512);
	if (sendto(s, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa1, client_length) == -1)
		fprintf(stderr, "success.\n");
	do{
		fd_set readfds; //fd_set is a type
		FD_ZERO(&readfds); //initialize
		FD_SET(s, &readfds); //put the socket in the set
		if (!(outfds = select(1, &readfds, NULL, NULL, &timeout)))
		{//timed out, return         
			if (sendto(s, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa1, client_length) == -1)
				fprintf(stderr, "success.\n");
		}
		if (outfds == 1)
		{
			memset(szbuffer, 0, 512);
			ibytesrecv = recvfrom(s, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa1, &client_length);
			request = "";
			for (int i = 0; szbuffer[i] != 0; i++)
				request += szbuffer[i];
			if (prevMsg == request && request != randomNum)
			{
				outfds = 2;
				string ss = randomNum;
				memset(szbuffer, 0, 512);
				memcpy(szbuffer, ss.c_str(), 512);
				if (sendto(s, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa1, client_length) == -1)
					fprintf(stderr, "success.\n");
			}
			else
				success = true;
		}
	} while (outfds != 1);

	client_Start_Seq = atoi(clientNum.c_str()) % 2;
	server_Start_Seq = atoi(randomNum.c_str()) % 2;
	Send_Seq = server_Start_Seq;
	next_Recv_Seq = client_Start_Seq;
	return success;
}
	void TcpServer::PutFileToClient(SOCKET ps1)
	{
		Fileinfo serverfile;
		Udppacket udpbuf;
	struct _stat stat_buf;
    int result;
	char filenamebuffer[260];
	char filebuffer[BUFFER_LENGTH];

	memset(filenamebuffer,0,sizeof(filenamebuffer));
//	int i = recv(ps1,filenamebuffer,sizeof(filenamebuffer),0);
//	int n = sizeof(filenamebuffer);
//	if(i!=n)
//		printf("Receive Req error,exit");
	
	udpbuf = udprecv(ps1,filenamebuffer,sizeof(filenamebuffer),0);
	strcpy(filenamebuffer,udpbuf.data);

	//cast it to the request packet structure		
	strcpy(serverfile.filename,filenamebuffer);
	if((result = _stat(serverfile.filename,&stat_buf))!=0)
		{
			sprintf(filenamebuffer,"No such a file\r");
//			if(send(ps1,filenamebuffer,sizeof(filenamebuffer),0)!=sizeof(filenamebuffer))
//				printf("send error\n");
			udpsend(ps1,filenamebuffer,sizeof(filenamebuffer),0);

		}		
		else 
		{
			serverfile.filelenth=stat_buf.st_size;
			//contruct the SUCCESS response and send it out
//			if(send(ps1,(char *)&serverfile,sizeof(serverfile),0)!=sizeof(serverfile))
//				printf("send error2\n");

			strcat(serverfile.filename,",");
			string str = to_string(serverfile.filelenth);
			strcat(serverfile.filename,str.c_str());
			udpbuf = udpsend(ps1,(char *)&serverfile,128,0);

			
			int nbsbyte =0; // sent bytes
			int sizeleft = (int) stat_buf.st_size; // size to send while buffer is less than sizeleft
			FILE *fp;
			fp = fopen (filenamebuffer,"rb");
		
			while (sizeleft >= 0)
			{
				if (sizeleft > BUFFER_LENGTH)
				{
					int byteread = fread(&filebuffer,1,BUFFER_LENGTH,fp);
//					if (send(ps1,filebuffer,BUFFER_LENGTH,0)!=BUFFER_LENGTH)
//					printf("Sending packet fail, exit\n");	

					udpbuf = udpsend(ps1,filebuffer,128,0);

					sizeleft = sizeleft - BUFFER_LENGTH;
				}
				else 
				{		
					int byteread=fread(&filebuffer,1,sizeleft,fp);		
//					if (send(ps1,filebuffer,sizeleft,0)!=sizeleft)
//					printf("Sending packet fail, exit");

					udpbuf = udpsend(ps1,filebuffer,128,sizeleft);

					sizeleft = sizeleft - BUFFER_LENGTH;
					fclose(fp);
				}
			}
			cout << "Upload Finished" << endl;
		}
		closesocket(ps1);
	}

void TcpServer::delFiles(SOCKET ds1)

 {
	Fileinfo serverfile;
	Udppacket udpbuf;
	struct _stat stat_buf;
    int result;
	char filenamebuffer[260];
	char filebuffer[BUFFER_LENGTH];

	memset(filenamebuffer,0,sizeof(filenamebuffer));
	udpbuf = udprecv(ds1,filenamebuffer,sizeof(filenamebuffer),0);
//	int n = sizeof(filenamebuffer);
//	if(i!=n)
//		printf("Receive Req error,exit");
	
	//cast it to the request packet structure		
	memcpy(filenamebuffer,udpbuf.data,sizeof(filenamebuffer));
	strcpy(serverfile.filename,filenamebuffer);
	if((result = _stat(serverfile.filename,&stat_buf))!=0)
		{
			char szlbuffer[260];
//			szlbuffer[0] = '#';
			memcpy(szlbuffer,"#\r",sizeof(szlbuffer));
			udpsend(ds1,szlbuffer,sizeof(szlbuffer),0);
//				printf("send error\n");

		}		
		else 
		{
/*			serverfile.filelenth=stat_buf.st_size;
			//contruct the SUCCESS response and send it out
			if(send(ds1,(char *)&serverfile,sizeof(serverfile),0)!=sizeof(serverfile))
				printf("send error2\n");
			
			int nbsbyte =0; // sent bytes
			int sizeleft = (int) stat_buf.st_size; // size to send while buffer is less than sizeleft
			
			FILE *fp;
			fp = fopen (serverfile.filename,"rb");
*/		
			if(!_access(filenamebuffer,0)){
				SetFileAttributes(filenamebuffer,0);
				if (DeleteFile(filenamebuffer))
				{
					char servbuffer[260];
//				servbuffer[0] = '@';
					memcpy(servbuffer,"@\r",sizeof(servbuffer));
				udpsend(ds1,servbuffer,sizeof(servbuffer),0);
//					printf("error in send in server program\n");				
			cout << "delete Finished" << endl;
				}
			}
		}
		closesocket(ds1);	
}

	void TcpServer::listFiles(SOCKET ss1)

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
				udpsend(ss1,serverfile.name,sizeof(serverfile.name),0);
//					printf("error in send in server program\n");
        }
    } while (!(_findnext(serverfileHandle, &serverfile)));
    _findclose(serverfileHandle);
				char servbuffer[260];
//				servbuffer[0] = '@';
				memcpy(servbuffer,"@\r",sizeof(servbuffer));
				udpsend(ss1,servbuffer,sizeof(servbuffer),0);
//					printf("error in send in server program\n");				
}

	int main(void){
	
		TcpServer tserver;
		tserver.run();
		
			
		closesocket(s1);		
		closesocket(s);
		/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
		WSACleanup();
		return 0;
	}




