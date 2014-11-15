/*FTP file Client

*/
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <iostream>
#include <string.h>
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tchar.h>
#include <direct.h>
#include <sstream>
#pragma comment( linker, "/defaultlib:ws2_32.lib" )

#define REQUEST_PORT 7000
#define BUFFER_LENGTH 260 
#define LISTEN_PORT 5000 //Listen port to send/receive files to/from
#define USEC 300000
#define SECT 0 
#define SIZE 256
using namespace std;

typedef struct
{
	int seqNo;
	char data[BUFFER_LENGTH];
}Udppacket;

typedef struct
{
		char filename[260];
		int filelenth;
}Fileinfo;

class TcpClient
{
    int sock;                    /* Socket descriptor */
	struct sockaddr_in ServAddr; /* server socket address */
	struct sockaddr_in sa_in;
	unsigned short ServPort;     /* server port */	
	WSADATA wsadata;
	int prev_Recv_Seq;
	int next_Rec_Seq;
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
	private:
	SOCKET mySocket;

	char szbuffer[SIZE];
	int server_length;
	int client_Start_Seq;
	int server_Start_Seq;
	int current_Recv_Seq;
	char localhost[21];
	char remotehost[21];

	HANDLE test;
	DWORD dwtest;

	char sz[SIZE];
	char fileSz[SIZE];

	struct timeval timeout1;
	char *fn;
	
	string filePath;
	string serv_size;

public:
	TcpClient(){}
	void connectserver(char servername[]);
	void listClientFiles();
	void listServerFiles();
	void DeleteServerFiles();
	void GetFileFromServer();
	void PutFileToServer();
	void closeconnect();
	void setsain();
	bool threeWayHandShake();
	Udppacket udpsend(SOCKET sock,char *sbuffer,int buflength, int sendflag);
	Udppacket udprecv(SOCKET sock,char *sbuffer,int buflength, int sendflag);
	~TcpClient();
};

void TcpClient::closeconnect()
{
	closesocket(sock);
}

Udppacket TcpClient::udpsend(SOCKET sock,char *sbuffer,int buflength, int sendflag)
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
		ibytesput = sendto(sock,(char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));
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

//					cout<<"recieve ack:"<<recvmsg.data<<endl;
/*					if(TRACE)
					{
						fout<<"Sender: received ACK for packet "<<Send_Seq<<endl;
					}
*/
					totalsendnumberpackets++;
				}
			}while(outfds!=1);
			Send_Seq= abs(Send_Seq-1);

	return sendmsg;
}



Udppacket TcpClient::udprecv(SOCKET sock,char *sbuffer,int buflength, int sendflag)
{
//	char recvbuffer[BUFFER_LENGTH];
	char sendbuffer[BUFFER_LENGTH];
	int msglength = sizeof(struct sockaddr_in);
	totalrecvnumberpackets = 0;
	totalrecvpacket = 0;

do{
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
            ack="clientACK"+out.str();

            if(current_Recv_Seq==next_Rec_Seq)
			{
				memset(sendmsg.data,0,BUFFER_LENGTH);
				memcpy(sendmsg.data,ack.c_str(),BUFFER_LENGTH);
				totalrecvpacket++;
                if (sendto(sock, (char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, BUFFER_LENGTH) == -1)
                    fprintf(stderr, "Error transmitting data.\n");	
//				cout<<"send ack:"<<sendmsg.data<<endl;
            }
            // else if seq number=previous seq number
            else 
			{               
				memset(sendmsg.data,0,BUFFER_LENGTH);
				memcpy(sendmsg.data,ack.c_str(),BUFFER_LENGTH);

                if (sendto(sock, (char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, BUFFER_LENGTH) == -1)
                    fprintf(stderr, "Error transmitting data.\n");		
            }


        }
    }while(current_Recv_Seq==prev_Recv_Seq);			
    prev_Recv_Seq=current_Recv_Seq;
    next_Rec_Seq= abs(next_Rec_Seq-1);     

	return recvmsg;
}


void TcpClient::PutFileToServer()
{
	Fileinfo clientfile;
	char tempclient[128];
	struct _stat putfilestate;
	char putfilename[260];
	char putfilebuffer[BUFFER_LENGTH];
	Udppacket udpbuf;
	int msglength = sizeof(udpbuf);
//	char recvbuffer[msglength];
//	char sendbuffer[msglength];


		Send_Seq = 0;
	next_Rec_Seq = 0;

	sprintf(tempclient,"PUT\r"); 

/*	int ibytesput = send(sock,tempclient,128,0);
	if (ibytesput == SOCKET_ERROR)
		throw "Send failed\n";  
	else
		cout << "Message to server: " << tempclient;
*/
//	memset(udpbuf.data,0,BUFFER_LENGTH);
	cout << "Message to server: " << tempclient <<endl;
	udpbuf = udpsend(sock,tempclient,128,0);

//	memset(sendmsg.data,0,BUFFER_LENGTH);
//	sendmsg.seqNo=Send_Seq;
//	memcpy(sendmsg.data,tempclient,BUFFER_LENGTH);
//	int ibytesput = send(sock,msg.data,128,0);


/*	int ibytesrecv=0; 
		if((ibytesrecv = recv(sock,tempclient,128,0)) == SOCKET_ERROR)
			throw "Receive failed\n";
		else
			cout << "Operation: " << tempclient;     
*/

	memset(udpbuf.data,0,BUFFER_LENGTH);
	udpbuf = udprecv(sock,tempclient,128,0);

	cout << "Operation: " << udpbuf.data <<endl; 


	cout<<endl
		<<"Please input the file name:"<<endl;
	cin>>putfilename;
		
	strcpy(clientfile.filename,putfilename);
	if( _stat(putfilename,&putfilestate)!=0)
		{
			printf("No such a file :%s\n",putfilename);
		}		
		else 
		{
			
			clientfile.filelenth=putfilestate.st_size;	
			strcat(clientfile.filename,",");

			string str = to_string(clientfile.filelenth);

			strcat(clientfile.filename,str.c_str());
			//contruct the SUCCESS response and send it out
//			if(send(sock,(char *)&clientfile,sizeof(clientfile),0)!=sizeof(clientfile))
//				printf("send error\n");
			udpbuf = udpsend(sock,(char *)&clientfile,128,0);
			
			int nbsbyte =0; // sent bytes
			int sizeleft = (int) putfilestate.st_size; // size to send while buffer is less than sizeleft
			FILE *fp;
			fp = fopen (putfilename,"rb");
		
			while (sizeleft >= 0)
			{
				if (sizeleft > BUFFER_LENGTH)
				{
					int byteread = fread(&putfilebuffer,1,BUFFER_LENGTH,fp);
//					if (send(sock,putfilebuffer,BUFFER_LENGTH,0)!=BUFFER_LENGTH)
//					printf("Sending packet fail, exit");	

					udpbuf = udpsend(sock,putfilebuffer,128,0);

					sizeleft = sizeleft - BUFFER_LENGTH;
				}
				else 
				{		
					int byteread=fread(&putfilebuffer,1,sizeleft,fp);		
//					if (send(sock,putfilebuffer,sizeleft,0)!=sizeleft)
//					printf("Sending packet fail, exit");

					udpbuf = udpsend(sock,putfilebuffer,128,sizeleft);

					sizeleft = sizeleft - BUFFER_LENGTH;
					fclose(fp);
				}
			}
			cout << "Upload Finished" << endl;
		}
}

void TcpClient::listClientFiles()

 {
	struct _finddata_t clientfile;
    long clientfileHandle;
	char clientdirbuffer[256];
	getcwd(clientdirbuffer,256);

    //string curPath";
	string curPath = strcat( clientdirbuffer,"\\*.*");

    if ((clientfileHandle = _findfirst(curPath.c_str(), &clientfile)) == -1) 
    {
        return;
    }    
	printf("Client File List\n");
	printf("********************************\n");
    do {
            if (_A_ARCH == clientfile.attrib) 
            {
               printf("%s\n", clientfile.name);
        }
    } while (!(_findnext(clientfileHandle, &clientfile)));
	printf("********************************\n");
    _findclose(clientfileHandle);
}


void TcpClient::listServerFiles()

 {
	char szbuffer[260];
	int ibufferlen=0;
	int ibytessent=0;
	int ibytesrecv=0;
	Udppacket udpbuf;
	
	 sprintf(szbuffer,"LIST\r"); 

		ibytessent=0;    
		ibufferlen = strlen(szbuffer);
		udpbuf = udpsend(sock,szbuffer,ibufferlen,0);
//		if (ibytessent == SOCKET_ERROR)
//			throw "Send failed\n";  
//		else
		cout << "Message to server: " <<udpbuf.data<<endl;
		memcpy(szbuffer,udpbuf.data,BUFFER_LENGTH);
		printf("Server File List\n");
		printf("********************************\n");
		szbuffer[0] = '\0';
		do
		{
			if(szbuffer[0] == '\0');
			else
			cout << "filename: " << szbuffer<<endl;  
			udpbuf = udprecv(sock,szbuffer,sizeof(szbuffer),0);
			memcpy(szbuffer,udpbuf.data,BUFFER_LENGTH);
//			throw "Receive failed\n";
				
		} while (szbuffer[0] != '@');
	 	printf("********************************\n");
}


void TcpClient::DeleteServerFiles()

 {
char getfilename[260];
char szzbuffer[260];
	char dtemp[260];
	Fileinfo clientfilename;
	char recvbuffer[BUFFER_LENGTH];
	int ibufferlen=260;
	int ibytessent=0;    
	int resu=0;
	Udppacket udpbuf;
	struct _stat filestate;

	sprintf(dtemp,"DEL\r"); 

	udpbuf = udpsend(sock,dtemp,sizeof(dtemp),0);
//	if (ibytessent == SOCKET_ERROR)
//		throw "Send failed\n";  
//	else
		cout << "Message to server: " << dtemp<<endl;

//	int ibytesrecv=0; 
		udpbuf = udprecv(sock,dtemp,128,0);
//			throw "Receive failed\n";
//		else
		cout << "Operation: " << udpbuf.data<<endl;     

	cout<<endl
		<<"Please input the file name:"<<endl;
	cin>>getfilename;

//		ibytessent=0;    
//		ibufferlen = sizeof(getfilename);
//		ibytessent = send(sock,getfilename,ibufferlen,0);
		udpbuf = udpsend(sock,getfilename,ibufferlen,0);
//		if (ibytessent == SOCKET_ERROR)
//			throw "Send failed\n";  
//		else
		cout << "DEL file from server: " << udpbuf.data<<endl;;

		//receive the response
		udpbuf = udprecv(sock,szzbuffer,sizeof(szzbuffer),0);
		memcpy(szzbuffer,udpbuf.data,sizeof(szzbuffer));
//			printf("recv response error,exit\n");

		//check if file exist on server, compare the names of filename and the response in respp structure
//		int ms = strcmp(getfilename, clientfilename.filename);

			if(szzbuffer[0] == '#')
			cout << "no such file"<<endl;
			else
			cout << "DEL file: ON THE SERVER"<<endl;  
//			if((ibytesrecv = recv(sock,szzbuffer,sizeof(szzbuffer),0)) == SOCKET_ERROR)
//			throw "Receive failed\n";
				
/*			printf("Response:file size %ld\n",clientfilename.filelenth);
			// get the filesize
			int fsize = clientfilename.filelenth;
			if((resu = _stat(getfilename,&filestate))==0)
				printf("Over Write local file:%s\n",getfilename);
			// start getting the file
			int nrbyte =0; // number of received bytes
			FILE *fp;
			fp = fopen (clientfilename.filename,"wb");
	
			cout << "Size of file to be received " << fsize << endl;

			while (fsize > 0)
			{
				if (fsize > BUFFER_LENGTH)
				{
					nrbyte = recv(sock,recvbuffer,BUFFER_LENGTH,0);
					fwrite(recvbuffer,1,nrbyte,fp);
				}
				else
				{
					nrbyte = recv(sock,recvbuffer,fsize,0);
					fwrite(recvbuffer,1,nrbyte,fp);
				}
	
				fsize = fsize - nrbyte;
	
			}
			fclose(fp);
			cout << "Download Finished" << endl;
*/		
}

void TcpClient::GetFileFromServer()
{
	char getfilename[260];
	char temp[128];
	Fileinfo clientfilename;
	char recvbuffer[BUFFER_LENGTH];
	int ibufferlen;
	int ibytessent=0;    
	int resu=0;
	Udppacket udpbuf;
	string con_msg,cli_dir;
	struct _stat filestate;

	sprintf(temp,"GET\r"); 

//	ibytessent = send(sock,temp,128,0);
//	if (ibytessent == SOCKET_ERROR)
//		throw "Send failed\n";  
//	else

	udpbuf = udpsend(sock,temp,sizeof(temp),0);
	cout << "Message to server: " << temp<<endl;

//	int ibytesrecv=0; 
//		if((ibytesrecv = recv(sock,temp,128,0)) == SOCKET_ERROR)
//			throw "Receive failed\n";
//		else
		udpbuf = udprecv(sock,temp,128,0);
		cout << "Operation: " << udpbuf.data;     

	cout<<endl
		<<"Please input the file name:"<<endl;
	cin>>getfilename;

/*		ibytessent=0;    
		ibufferlen = sizeof(getfilename);
		ibytessent = send(sock,getfilename,ibufferlen,0);
		if (ibytessent == SOCKET_ERROR)
			throw "Send failed\n";  
		else
*/
		udpbuf = udpsend(sock,getfilename,sizeof(getfilename),0);
		cout << "Get file from server: " << getfilename<<endl;;

		//receive the response
//		if(recv(sock,(char *)&clientfilename,sizeof(clientfilename),0)!=sizeof(clientfilename))
//			printf("recv response error,exit\n");
		udpbuf = udprecv(sock,temp,128,0);

		for (int i = 0; udpbuf.data[i] != 0; i++)
		con_msg += udpbuf.data[i];
		int pos = con_msg.find_first_of(',');
		cli_dir = con_msg.substr(0, pos);
		con_msg = con_msg.substr(pos + 1);

		
		strcpy(clientfilename.filename,cli_dir.c_str());
		clientfilename.filelenth = atoi(con_msg.c_str());

		//check if file exist on server, compare the names of filename and the response in respp structure
		int ms = strcmp(getfilename, clientfilename.filename);
		if(ms == 0)
		{
			printf("Response:file size %ld\n",clientfilename.filelenth);
			// get the filesize
			int fsize = clientfilename.filelenth;
			if((resu = _stat(getfilename,&filestate))==0)
				printf("Over Write local file:%s\n",getfilename);
			// start getting the file
			int nrbyte =0; // number of received bytes
			FILE *fp;
			fp = fopen (clientfilename.filename,"wb");
	
			cout << "Size of file to be received " << fsize << endl;

			while (fsize > 0)
			{
				if (fsize > BUFFER_LENGTH)
				{
					udpbuf = udprecv(sock,(char *)&clientfilename,sizeof(clientfilename),0);
					memcpy(recvbuffer,udpbuf.data,sizeof(udpbuf.data));
					nrbyte = sizeof(udpbuf.data);
					fwrite(recvbuffer,1,nrbyte,fp);
				}
				else
				{
					udpbuf = udprecv(sock,(char *)&clientfilename,sizeof(clientfilename),0);
					memcpy(recvbuffer,udpbuf.data,sizeof(udpbuf.data));
					nrbyte = fsize;
					fwrite(recvbuffer,1,nrbyte,fp);
				}
	
				fsize = fsize - nrbyte;
	
			}
			fclose(fp);
			cout << "Download Finished" << endl;
		}	
		else
		{
			printf("No such file on the server");
		}
}


void TcpClient::connectserver(char servername[])
{
	Send_Seq = 0;
	next_Rec_Seq =0;
	prev_Recv_Seq = 3000;
	if (WSAStartup(0x0202,&wsadata)!=0)
	{  
		WSACleanup();  
		 printf("Error in starting WSAStartup()\n");
	}

	struct hostent *p;
	if((p = gethostbyname(servername))==NULL)
		{
			printf("Host not exist:%s\n",servername);
//			exit(0);
	}

	if((sock = socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET)
			throw "Socket failed\n";

/*
		DWORD dwBytesReturned = 0;
		BOOL bNewBehavior = FALSE;
		DWORD status;
		status = WSAIoctl(mySocket, SIO_UDP_CONNRESET,&bNewBehavior,sizeof (bNewBehavior),NULL, 0, &dwBytesReturned,NULL, NULL);
		if (SOCKET_ERROR == status)
		{
		DWORD dwErr = WSAGetLastError();
		if (WSAEWOULDBLOCK == dwErr)
			{
			}
		else
			{
		printf("WSAIoctl(SIO_UDP_CONNRESET) Error: %d/n", dwErr);
			}
		}
*/

//	struct in_addr in;
//    memcpy(&in,p->h_addr_list[0],sizeof(struct in_addr));
	ServPort=LISTEN_PORT;
	memset(&ServAddr, 0, sizeof(ServAddr));     /* Zero out structure */
	ServAddr.sin_family      = AF_INET;             /* Internet address family */
	ServAddr.sin_addr.s_addr = *((unsigned long *) p->h_addr_list[0]);//ResolveName(servername);   /* Server IP address */
	ServAddr.sin_port        = htons(ServPort); /* Server port */
	
			//Create the socket
/*	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) //create the socket 
		{
			printf("Socket Creating Error");
			exit;
		}
*/
	if (bind(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
		{
			printf("Socket Creating Error \n");
		}
	else
		printf("connect to server %s :%d \n",servername,ServAddr.sin_port);

}

void TcpClient::setsain()
{
		char remotehost[256];		
		struct hostent *rp;
  				cout << "please enter your router name :" << flush ;
			
				cin >> remotehost ;
			cout << "Router name is: \"" << remotehost << "\"" << endl;
			
			if((rp=gethostbyname(remotehost)) == NULL)
			{
			cout<< "remote gethostbyname failed\n";
			//continue;
			}
			memset(&sa_in,0,sizeof(sa_in));
			memcpy(&sa_in.sin_addr,rp->h_addr,rp->h_length);
			sa_in.sin_family = rp->h_addrtype;
			sa_in.sin_port = htons(REQUEST_PORT);
			//Display the host machine internet address
			cout << "Connecting to remote host:";
			cout << inet_ntoa(sa_in.sin_addr) << endl;

}
bool TcpClient::threeWayHandShake()
{
	bool success=true;
	//string prevMsg;
	int ibytesrecv=0;	
	string ss;
	string randomNum;
	srand(1);
	int random_integer = rand() % 256;
	stringstream out;
	out.str("");
	out<<random_integer;
	randomNum=out.str();	
	ss = randomNum;	
	memset(szbuffer,0,SIZE);
	memcpy(szbuffer,ss.c_str(),SIZE);
	if (sendto(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, server_length) == -1)
		fprintf(stderr, "success.\n");
	do{
		fd_set readfds; //fd_set is a type
		FD_ZERO(&readfds); //initialize
		FD_SET(mySocket, &readfds); //put the socket in the set
		if(!(outfds = select (1 , &readfds, NULL, NULL, & timeout))) 
		{//timed out, return
			if (sendto(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, server_length) == -1)
				fprintf(stderr, "success.\n");
		}
		if (outfds == 1) 
		{
			memset(szbuffer,0,SIZE);
			ibytesrecv = recvfrom(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, &server_length);
			cout<<szbuffer<<endl;
		}
	}while(outfds!=1);

	//get server number and send as acknowledgement
	string request,clientNum,serverNum;
	for(int i = 0; szbuffer[i] != 0; i++)
		request += szbuffer[i];
	serverNum = request;
	
	memset(szbuffer,0,SIZE);
	memcpy(szbuffer,serverNum.c_str(),SIZE);
	if (sendto(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, server_length) == -1)
		fprintf(stderr, "success.\n");
	do{
		fd_set readfds; //fd_set is a type
		FD_ZERO(&readfds); //initialize
		FD_SET(mySocket, &readfds); //put the socket in the set
		if(!(outfds = select (1 , &readfds, NULL, NULL, & timeout))) 
		{//timed out, return
			success=true;
			outfds=2;
		}
		else if (outfds == 1) 
		{
			memset(szbuffer,0,SIZE);
			ibytesrecv = recvfrom(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, &server_length);
			
			memset(szbuffer,0,SIZE);
			memcpy(szbuffer,serverNum.c_str(),SIZE);
			if (sendto(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, server_length) == -1)
				fprintf(stderr, "success.\n");
		}
	}while(success!=true);
	server_Start_Seq=atoi(serverNum.c_str())%2;
	client_Start_Seq=atoi(randomNum.c_str())%2;
	Send_Seq=client_Start_Seq;
	next_Rec_Seq=server_Start_Seq;
	return success;
}


TcpClient::~TcpClient()
{
	/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
	WSACleanup();  
}

int main()
{

	TcpClient *tc=new TcpClient();
	
	char servername[256];
	char filename[256];
	char command[256];


	  cout << "Type name of ftp server: ";
	  cin >> servername;
	  tc->connectserver(servername);
	  tc->setsain();
  do
  {
	  cout<< endl
			<< "Please choose the option below"     << endl	
			<< "1 List file from client"			<< endl
			<< "2 List file from server "			<< endl
			<< "3 Get file from server"				<< endl
			<< "4 Put file from server "			<< endl
			<< "5 Delete file from server "			<< endl
			<< "Please input 1-5 "							<< endl;
	  cin >> command;
	  
		if (strcmp(command,"1")==0)
		{
			tc->listClientFiles();
		}
			if (strcmp(command,"2")==0)
		{
//			tc->connectserver("localhost");
//			tc->setsain(servername);
			
			tc->listServerFiles();
			
		}
			if (strcmp(command,"3")==0)
		{
//			 tc->connectserver("localhost");
//			tc->setsain(servername);
			
			 tc->GetFileFromServer();
		}
			if (strcmp(command,"4")==0)
		{
//			 tc->connectserver("localhost");
//			 tc->setsain(servername);
			
			 tc->PutFileToServer();
		}
			if (strcmp(command,"5")==0)
		{
//			tc->connectserver("localhost");
//			tc->setsain(servername);
			
			tc->DeleteServerFiles();
		}
;
  } while (true);
  tc->closeconnect();
  return 0;
}
