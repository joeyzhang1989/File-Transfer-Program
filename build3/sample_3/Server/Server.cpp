/**************************************************************************/
/* 			Comp 445				                                      */
/*			Lab Assignment #3		                                      */
/*           ------------------                                            */
/* Reference: Sample provided on course webpage							  */
/* Team Info:                                                             */   
/*              Mohammad Ali ID# 5584493  Email  aliraja4@gmail.com       */
/*			    Zeeshan MIr  ID# 5525977  Email  zeesh_mir@hotmail.com    */
/**************************************************************************/

#include <io.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winsock.h>
#include <time.h>
#include <fstream>
#pragma comment(lib,"wsock32.lib") //for linker

#define ROUTER_PORT1 7000			//router port number 1
#define ROUTER_PORT2 7001			//router port number 2
#define PEER_PORT1 5000				//port number of peer host 1
#define PEER_PORT2 5001				//port number of peer host 2

//Port data types
#define REQUEST_PORT 0x7070
int port=REQUEST_PORT;

//socket data types
SOCKET s;
SOCKADDR_IN saddr;      // filled by bind
SOCKADDR_IN caddr;     // fill with server info, IP, port
union {
	struct sockaddr generic;
	struct sockaddr_in ca_in;
}ca;

const char SWS = '3';	
const int dataSize = 80;
const int packetSize = 90;

int calen=sizeof(ca); 
const char* logfile = "server_log.log";// server log file
int rate = 2;
int k = 4;
//buffer data types
	
char client_info[packetSize]; //for holding client infomation
char packet[packetSize]; //for holding file package
char flag[2];	//for verifing if the file exists 
char NackBuffer[8]; //for holding NAck structure
const int windowSize = SWS - 48;
char windowBuffer[windowSize][packetSize];
char *buffer;
int ibufferlen;
int ibytesrecv;	
int ibytessent;	
int caddrLen;

//host data types
char localhost[21];
HOSTENT *hp; //information for server
//HOSTENT *hs; //information for client

//wait variables
int nsa1;
int r,infds=1, outfds=0;
struct timeval timeout;
const struct timeval *tp=&timeout;
fd_set readfds;

//others
HANDLE test;
DWORD dwtest;

//Data structure for holding received data
struct received_data 
{
	char filename[35];
	char sender[11];
	char direction[4];
	char remotehost[21];
	int dropRate;
};

struct packet_data 
{
	char wSeq;
	int seqNumber;
	char dataBuffer[dataSize];
};

struct NAck
{
	// n->Nak a->Ack
	char index;
	//0 to 7
	char wSeq;  
	int seq;
};


/*------------------------------------*/
/* user-function			          */
/*------------------------------------*/

int send_file(char sfile[]);
int receive_file(char dfile[]);
void clearbuf(char* buf, int size);
bool dropPacket();
void sendWindow(int, int);
int receive_NAck();

using namespace std;

/*--------------------------------------------------------------------*/
/* Method:			main()									          */
/*--------------------------------------------------------------------*/
int main(void)
{

	//pointer rec_data points to received data
 	received_data* rec_data;  
	WSADATA wsadata;

	try{        		 
        if (WSAStartup(0x0202,&wsadata)!=0)
		{  
			WSACleanup();  
			cout<<"Error in starting WSAStartup()\n";
        }
		//else
	//	{
			//buffer="WSAStartup was suuccessful\n";   
			//WriteFile(test,buffer,sizeof(buffer),&dwtest,NULL); 
      //  }  
		
		//Display info of local host
		cout<<"\nFile Transfer Protocol Server" << endl;
		cout<<"============================================================" << endl;
		cout<<"" << endl;
		cout<<"" << endl;


		gethostname(localhost,21);
		cout<<"hostname: "<<localhost<< endl;
		
		if((hp=gethostbyname(localhost)) == NULL) 
		{
		   cout << "gethostbyname() cannot get local host info!"
		        << WSAGetLastError() << endl; 
		}

        //Create the server socket
		if((s = socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET) 
			throw "can't initialize socket";

		//Fill-in Server Port and Address info.
        saddr.sin_family = AF_INET;
		//saddr.sin_port = htons(port);
		saddr.sin_port = htons(PEER_PORT2); 
		saddr.sin_addr.s_addr = htonl(INADDR_ANY);  
		
		//Bind the server port
		if (bind(s,(LPSOCKADDR)&saddr,sizeof(saddr)) == SOCKET_ERROR)
         throw "Can't bind the socket";


		FD_ZERO(&readfds);
		
		cout << "ftpd_tcp starting at host:[" << localhost <<"]"<<endl;
		cout << "waiting to be contacted for transferring files..." << endl;
		//wait loop

		while(1){		
 
		caddrLen = sizeof(caddr);
 		//Fill in client_info from accepted request.
		rec_data = (received_data*) client_info;
		if ((ibytesrecv = recvfrom(s, (char*)client_info, sizeof(received_data), 0, (struct sockaddr *)&caddr, &caddrLen)) < 0)
				throw "Receive error in server program\n";
		
		flag[0] = 'y';
		if ((ibytessent = sendto(s, flag, sizeof(flag), 0, (struct sockaddr *)&caddr, caddrLen)) != sizeof(flag))
				throw "sendto() send a different number of bytes than expected";
	
		cout<<"Accepted Connection From "<<rec_data->sender<<"."<<endl;
		rate = rec_data->dropRate;

		//check whether user enter "get", "put", or wrong command

		if (strcmp(rec_data->direction, "get") == 0) 
		{   //client sending "get" command
			cout << "User '" << rec_data->sender << "' requesting file with file name " << rec_data->filename << endl;
			cout << "Sending file to " << rec_data->sender <<". waiting..." << endl;
			
			if (send_file(rec_data->filename) == -1)
				throw "Unable to send file to client!\n";
		}

		else if (strcmp(rec_data->direction, "put") == 0) 
		{	//client sending "put" command
			cout << "\nReceiving file " << rec_data->filename << " from " << rec_data->sender << "." << endl;
			
			if (receive_file(rec_data->filename) == -1)
				throw "Unable to recieve file from client!\n";
		}
		
		else cout << "Wrong request command" <<  rec_data->direction << endl; //client sending wrong command
		
		cout << "Return to waiting state." <<endl<<endl;
		
		}//wait loop
		system("pause");
	} //try loop

 	//Display needed error message.
	catch(char* str) { cerr<<str<<WSAGetLastError()<<endl;}

	//close Client socket
	closesocket(s);		

 	/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
   WSACleanup();
   return 0;
}




/*------------------------------------*/
/* function	   send ACK 	          */
/*------------------------------------*/



int send_NAck()
{
	ibytessent = 0;
	if((ibytessent = sendto(s, NackBuffer, sizeof(NackBuffer), 0, (struct sockaddr *)&caddr, caddrLen)) == SOCKET_ERROR)	{
		cout << "Error in send in server program" << endl;
		return -1;
	}
	return 0;
}

/*------------------------------------*/
/* function	  receive ACK 	          */
/*------------------------------------*/


int receive_NAck()
{
	ibytesrecv = 0; 
	if((ibytesrecv = recvfrom(s, NackBuffer, sizeof(NackBuffer), 0, (struct sockaddr *)&caddr, &caddrLen)) == SOCKET_ERROR)	
	{
		cout << "Error in send in server program" << endl;
		return -1;
	}
	return 0;
}

/*------------------------------------*/
/* function	   receiver flag 	      */
/*------------------------------------*/

int rec_flag()
{
    ibytesrecv = recvfrom(s, flag, sizeof(flag), 0, (struct sockaddr *)&caddr, &caddrLen);
    if(ibytesrecv < 0)      {
	    cout << "Error in receive in client program" << endl;
        return -1;
    }
	return 0;
}

/*------------------------------------*/
/* function	   send flag 	          */
/*------------------------------------*/
int send_flag()
{
	ibytessent = 0;

	if((ibytessent = sendto(s, flag, sizeof(flag), 0, (struct sockaddr *)&caddr, caddrLen)) == SOCKET_ERROR)	
	{
		cout << "Error in send in server program" << endl;
		return -1;
	}

	return 0;
}

/*------------------------------------*/
/* function	   send file	          */
/* implement   PUT command	          */
/*------------------------------------*/

int send_file(char sfile[]) 
{
	ibytessent = 0; 
	FILE* stream;		
	ofstream output(logfile);
	int handle;
	long filesize;
	timeout.tv_sec = 0;
	timeout.tv_usec = 300000;

	int ibytelen;

	int currentSeq = 0;
	int extraBytes = 0;	
	int lastACK = 0;
	int currentACK = 0;
	int start = 0;					//the oldest window frame
	int end = 0;					//the newest window frame
	int currentNAK = 0;
	char winSeq = '0';

	//verify if file exists on the server
	if ((fopen(sfile, "r")) ==NULL)	
	{
		flag[0] = 'n';

		//the file does not exists stop transfer
		send_flag();

		cout <<  "\nFile can not found. Transfer of \"" << sfile << "\" stop!\n" << endl;
		return 0;
	}

	else	
	{
		flag[0] = 'y';

		//continue if file does exist
		send_flag();
	}

	//waitting for a response from the client 
	rec_flag();

	if(flag[0] == 'o')	
	{
		cout << "The client cannot creat destination file.  Transfer aborted!" << endl;
		return 0;
	}

	if(flag[0] == 'n')	
	{
		cout << "The client already have the file.  Transfer aborted!" << endl;
		return 0;
	}

	
	ibytelen = dataSize;

		
	if ((stream = fopen(sfile, "rb")) == NULL) 
	{
			
		cout << "File not found or unable to open file!" << endl;		
		return 0;
		
	}
        
	// get the handle number of the file
	handle = _fileno(stream);

	// get the file size of the file
	filesize = _filelength(handle);

	cout << "File size is " << filesize << " bytes." << endl;
	//set timer
	srand( (unsigned)time( NULL ) ); 
	packet_data* pack;
	pack = (packet_data*) packet;
	packet_data* window[windowSize];
	
	for(int j=0; j<windowSize; j++)
		window[j] = (packet_data*) windowBuffer[j];
	NAck* AckNak;
	AckNak = (NAck*) NackBuffer;
	
	int numofPackets = (int)(filesize/dataSize) + 1;
	
	cout << "Number of packets needed is " << numofPackets << "." << endl;
	
	int lastPacketSize = filesize % dataSize;	
	
	
	cout << "Waiting for tranfer to complete ..." << endl;
	
	//set up the first window
	cout<<"window size = "<<windowSize<<endl;

	for(int i=0; i<windowSize; i++) 
	{
		if(currentSeq < numofPackets) 
		{
			fread(pack->dataBuffer,sizeof(char),ibytelen,stream);
			pack->wSeq = winSeq;
			currentSeq++;
			pack->seqNumber = currentSeq;

			if(currentSeq == numofPackets) 
				pack->seqNumber = -1*lastPacketSize;
			ibytessent = 0;
			
			for(int j=0; j<sizeof(packet_data); j++)
				windowBuffer[i][j] = packet[j];
			
			cout<<"Sender: sent out packet "<<window[i]->seqNumber<<" with window sequence "<<window[i]->wSeq<<"."<<endl;
			output<<"Sender: sent out packet "<<window[i]->seqNumber<<" with window sequence "<<window[i]->wSeq<<"."<<endl;
			winSeq++;
			
			if(winSeq > SWS) 
				winSeq ='0';
			end++; 
		}
	}
	
	end--;
	lastACK = end;
	currentACK = end;

	while(currentSeq < numofPackets) 
	{
		sendWindow(start, end);
		bool loop = true;

		while(loop == true)
		{
			FD_SET(s, &readfds);

			if(!(outfds=select(infds,&readfds,NULL,NULL,&timeout))) 
			{
				sendWindow(start, end);
				cout << "Sender: sent window agian"<<endl;
				continue;
			}

			else if (outfds == SOCKET_ERROR)
				throw "failure in Select";
			else if (FD_ISSET(s,&readfds))	
				loop = true;
			
			FD_CLR(s, &readfds);
			receive_NAck();
			bool validACK = false;
			
			for( k=0; k<windowSize; k++)
			{
				if(window[k]->wSeq==AckNak->wSeq && window[k]->seqNumber==AckNak->seq) 
				{
					validACK = true;
					break;
				}
			}

			if(!dropPacket()) 
			{
				if(AckNak->index == 'a')
				{		
					//if ACK
					loop = true;
					if(validACK == true)
					{
						currentACK = k;
					
						cout<< "Sender: received an ACK for packet " << window[currentACK]->seqNumber 
							<< " with window sequence "<<window[currentACK]->wSeq<<"."<<endl;
						
						output<< "Sender: received an ACK for packet " << window[currentACK]->seqNumber 
							  << " with window sequence "<<window[currentACK]->wSeq<<"."<<endl;
						
						start = lastACK + 1;
						if(start > windowSize-1) start = 0;
						end = currentACK;
						lastACK = currentACK;
						int i = start;
						
						while(i != end) 
						{
							if(currentSeq < numofPackets) 
							{
								fread(pack->dataBuffer,sizeof(char),ibytelen,stream);
								pack->wSeq = winSeq;
								currentSeq++;	
								pack->seqNumber = currentSeq;

								if(currentSeq == numofPackets) 
									pack->seqNumber = -1*lastPacketSize;
								
								ibytessent = 0;
								
								for(int j=0; j<sizeof(packet_data); j++)
									windowBuffer[i][j] = packet[j];
								
								cout<<"Sender: sent out packet "<<window[i]->seqNumber<<" with window sequence "
									<<window[i]->wSeq<<"."<<endl;
								
								output<<"Sender: sent out packet "<<window[i]->seqNumber<<" with window sequence "
								      <<window[i]->wSeq<<"."<<endl;
								
								winSeq++;

								if(winSeq > SWS) 
									winSeq='0';
								i++;
								
								if(i > windowSize-1) 
									i = 0;
							}

							else break;
						}

						if(currentSeq < numofPackets) 
						{
							fread(pack->dataBuffer,sizeof(char),ibytelen,stream);
							pack->wSeq = winSeq;
							currentSeq++;
							pack->seqNumber = currentSeq;	

							if(currentSeq == numofPackets) 
								pack->seqNumber = -1*lastPacketSize;
					
							for(int j=0; j<sizeof(packet_data); j++)
								windowBuffer[i][j] = packet[j];

							cout<<"Sender: sent out packet "<<window[i]->seqNumber<<" with window sequence "
								<<window[i]->wSeq<<"."<<endl;
							
							output<<"Sender: sent out packet "<<window[i]->seqNumber<<" with window sequence "
							      <<window[i]->wSeq<<"."<<endl;
							
							winSeq++;
							
							if(winSeq > SWS) 
								winSeq='0';
						}

						loop = false;
					}
				}

				else if(AckNak->index == 'n') 
				{
					loop = true;
					if(validACK == true) 
					{
						currentNAK = k;
						start = currentNAK;
						end = lastACK;
						cout<< "Sender: received an NAK for packet " << window[currentNAK]->seqNumber 
							<< " with window sequence "<<window[currentNAK]->wSeq<<"."<<endl;
						
						output<< "Sender: received an NAK for packet " << window[currentNAK]->seqNumber 
							  << " with window sequence "<<window[currentNAK]->wSeq<<"."<<endl;
						
						loop = true;
					}
				}
			}

			else 
			{
				if(AckNak->index == 'a' && validACK == true) 
				{
					cout<<"Sender: dropped an ACK for packet " << window[k]->seqNumber 
						<< " with window sequence "<<window[k]->wSeq<<"."<<endl;
					
					output<< "Sender: dropped an ACK for packet " << window[k]->seqNumber 
						  << " with window sequence "<<window[k]->wSeq<<"."<<endl;
				}
				
				else if(AckNak->index == 'n' && validACK == true) 
				{
					cout<< "Sender: dropped an NAK for packet " << window[k]->seqNumber 
						<< " with window sequence "<<window[k]->wSeq<<"."<<endl;
					
					output<< "Sender: dropped an NAK for packet " << window[k]->seqNumber 
						  << " with window sequence "<<window[k]->wSeq<<"."<<endl;
			
				}
				
				loop = true;
			}
		}
	}

	//send the last window separately	
	sendWindow(start, end);
	int maxTimeOut = 20;
	bool loop = true;
	
	while(loop == true) 
	{
		FD_SET(s, &readfds);
		if(!(outfds=select(infds,&readfds,NULL,NULL,&timeout))) 
		{
			maxTimeOut --;
			if(maxTimeOut <= 0) 
			{
				cout << "Timeouts over limit!"<< endl;
				cout << "File Transfer terminated due to network failure!"<<endl;
				output << "Timeouts over limit!"<< endl;
				output << "File Transfer terminated due to network failure!"<<endl;
				return -1;
			}

			sendWindow(start, end);
			cout << "Sender: sent window agian"<<endl;
			continue;
		}

		else if (outfds == SOCKET_ERROR) 
			throw "failure in Select";
		
		else if (FD_ISSET(s,&readfds))	
			loop = true;
	
		FD_CLR(s, &readfds);
		receive_NAck();
		bool validACK = false;
		
		for(int k=0; k<windowSize; k++) 
		{
			if(window[k]->wSeq==AckNak->wSeq && window[k]->seqNumber==AckNak->seq) 
			{
				validACK = true;
				break;
			}
		}
		if(!dropPacket()) 
		{
			if(AckNak->index == 'a') 
			{
				if(validACK == true) 
				{
					currentACK = k;
					end = currentACK;
				
				}
				
				cout<< "Sender: last window received an ACK for packet " << window[currentACK]->seqNumber 
					<< " with window sequence "<<window[currentACK]->wSeq<<"."<<endl;
					
				output<< "Sender: last window received an ACK for packet " << window[currentACK]->seqNumber 
					  << " with window sequence "<<window[currentACK]->wSeq<<"."<<endl;
					
				loop = true;
				
			}
			
			if(AckNak->wSeq==window[lastACK]->wSeq && AckNak->seq==window[lastACK]->seqNumber) 
			{
			
				cout<< "File transfer completed successfully!"<<endl;
				
				output<< "File transfer completed successfully!"<<endl;
					
				fclose(stream);
				
				output.close();
				
				return 0;
				
			}
			
			else if(AckNak->index == 'n')
			{
				loop = true;
				
				if(validACK == true) 
				{
					currentNAK = k;
					start = currentNAK;
					end = lastACK;

					cout<< "Sender: last window received an NAK for packet " << window[currentNAK]->seqNumber 
						<< " with window sequence "<<window[currentNAK]->wSeq<<"."<<endl;
					
					output<< "Sender: last window received an NAK for packet " << window[currentNAK]->seqNumber 
						  << " with window sequence "<<window[currentNAK]->wSeq<<"."<<endl;
				
					loop = true;
				}
			}
		}

		else 
		{
			if(AckNak->index == 'a' && validACK == true) 
			{
				cout<< "Sender: last window dropped an ACK for packet " << window[k]->seqNumber 
					<< " with window sequence "<<window[k]->wSeq<<"."<<endl;
				
				output<< "Sender: last window dropped an ACK for packet " << window[k]->seqNumber 
					  << " with window sequence "<<window[k]->wSeq<<"."<<endl;
			
			}
			
			else if(AckNak->index == 'n' && validACK == true) 
			{
				cout<< "Sender: last window dropped an NAK for packet " << window[k]->seqNumber 
					<< " with window sequence "<<window[k]->wSeq<<"."<<endl;
				
				output<< "Sender: last window dropped an NAK for packet " << window[k]->seqNumber 
					  << " with window sequence "<<window[k]->wSeq<<"."<<endl;
			}
			
			loop = true;
		}
	}

	fclose(stream);
	output.close();
	cout << "\nTransfer of file to server completed!!" << endl;
	return 0;
}



/*------------------------------------*/
/* function	   receive file	          */
/* implement   GET command	          */
/*------------------------------------*/

int receive_file(char dfile[]) 
{

		FILE* stream;
		ofstream output(logfile);
        ibufferlen = dataSize;
	
		int counter = 0;
		int lastSeq = 0;
		char lastWSeq = SWS;
		int finalPacket = 0;
		bool nakflag = true;
	
		//receive answer from server
		ibytesrecv = 0;
		rec_flag();

		//verify if there is an error on the transfer of the answer
		if((flag[0] != 'n') && (flag[0] != 'y'))
			throw "Error in data transfer\n";

		//stop transfer if file not found on client
		if(flag[0] == 'n')	{
			cout << endl << "The client cannot find the source file. receiving stop!\n\n";
			return 0;
		}

		//continue if file found on the client, verify if the file exists in the client
		ibytessent = 0;
		flag[0] = ' ';

		//continue if file does not exists in server
		if((fopen(dfile, "r")) == NULL) 
		{
			if(fopen(dfile,"w") != NULL)	
			{
				flag[0] = 'y';
				send_flag();
			}
			
			else 
			{
				flag[0] = 'o';
				send_flag();
				cout<<"Desination file cannot be created."<<endl;
				return 0;
			}
		}

		//file exists in server
		else	
		{

			flag[0]='n';
			send_flag();
		}
		
		
		rec_flag();
		
		//if answer is no abort transfer 
		if(flag[0]=='n')	
		{
			cout << "Transfermation aborted." << endl;
			return 0;
		}
		
		if (flag[0]!='y' && flag[0]!='n')
		{
			cout << "client sends a wrong answer "<<flag[0] << endl;
			return 0;
		}
	
		//open file for writing
		if ((stream = fopen(dfile, "wb")) == NULL) 
		{
			cout << "Unable to open file in server!" << endl;
			return 0;
		}
	
	
		srand( (unsigned)time( NULL ) ); 
		NAck* nack;	
		nack = (NAck*) NackBuffer;
		packet_data* pack;
		pack = (packet_data*) packet;

	
		//wait for reception of server response.
		while (1) 
		{
		
			//clear the buffer		
			clearbuf(pack->dataBuffer, dataSize);        
			ibytesrecv=0; 		
			ibytesrecv = recvfrom(s,packet,sizeof(packet_data),0, (struct sockaddr *)&caddr, &caddrLen);
        
			if (ibytesrecv == SOCKET_ERROR) 
				throw "Receive file failed\n";
		
			if(!dropPacket()) 
			{
			
				char currentWSeq = lastWSeq + 1;
			
				if(currentWSeq > SWS) 
					currentWSeq = '0';
			
				if (pack->wSeq == currentWSeq && pack->seqNumber <= 0)
				{		
					//last dataBuffer size less than buffer size
					finalPacket = -1*pack->seqNumber;
					fwrite(pack->dataBuffer,sizeof(char),finalPacket,stream);					
					nack->index='a'; 
					nack->wSeq=pack->wSeq;
					nack->seq=pack->seqNumber;
					ibytessent = 0;
					send_NAck();
					cout<< "Receiver: received packet " << pack->seqNumber 
						<< " with window sequence "<<pack->wSeq<<"."<<endl;
				
					cout<< "Receiver: sent an ACK for packet " << pack->seqNumber 
						<< " with window sequence "<<pack->wSeq<<"."<<endl;
				
					output<< "Receiver: received packet " << pack->seqNumber 
						  << " with window sequence "<<pack->wSeq<<"."<<endl;
				
					output<< "Receiver: sent an ACK for packet " << pack->seqNumber 
						  << " with window sequence "<<pack->wSeq<<"."<<endl;
				
					output<< "\nTransfer of file from the client completed\n";
				
					cout<< "\nTransfer of file from the client completed\n";
				
					fclose(stream);	
				
					output.close();
			
					return 0;
			}

			else if (pack->wSeq == currentWSeq && pack->seqNumber == lastSeq+1) 
			{
				counter = 0;
				fwrite(pack->dataBuffer,sizeof(char),dataSize,stream);
				nack->index='a'; 
				nack->wSeq=pack->wSeq;
				nack->seq=pack->seqNumber;
				ibytessent = 0;
				send_NAck();
				lastWSeq = pack->wSeq;
				lastSeq = pack->seqNumber;

				cout<< "Receiver: received packet " << pack->seqNumber 
					<< " with window sequence "<<pack->wSeq<<"."<<endl;
				
				cout<< "Receiver: sent an ACK for packet " << pack->seqNumber 
					<< " with window sequence "<<pack->wSeq<<"."<<endl;
				
				output<< "Receiver: received packet " << pack->seqNumber 
					  << " with window sequence "<<pack->wSeq<<"."<<endl;
				
				output<< "Receiver: sent an ACK for packet " << pack->seqNumber 
					  << " with window sequence "<<pack->wSeq<<"."<<endl;
				
				
			}

			else 
			{
				counter++;
				if(counter > windowSize) 
				{
					counter = 0;
					nack->index='n';
					nack->wSeq=currentWSeq;
					nack->seq=lastSeq+1;
					ibytessent = 0;
					send_NAck();

					cout<< "Receiver: sent an NAK for packet " << lastSeq+1 
						<< " with window sequence "<<currentWSeq<<"."<<endl;
					
					output<< "Receiver: sent an NAK for packet " << lastSeq+1
						  << " with window sequence "<<currentWSeq<<"."<<endl;
				}
			} 
		}
	}

	fclose(stream);
	output.close();
	return 0;	
}


/*------------------------------------*/
/* function	   send Window	          */
/*------------------------------------*/
void sendWindow(int head, int tail) 
{
	int i = head;
	while(i != tail) 
	{
		ibytessent = 0;
		ibytessent = sendto(s, windowBuffer[i], sizeof(packet_data), 0, (struct sockaddr *)&caddr, caddrLen);
		if((ibytessent == SOCKET_ERROR))
			throw "Error in send in server program\n";
		i++;
		if(i > windowSize-1) i = 0;
	}

	ibytessent = 0;
	ibytessent = sendto(s, windowBuffer[i], sizeof(packet_data), 0, (struct sockaddr *)&caddr, caddrLen);

	if((ibytessent == SOCKET_ERROR))
		throw "Error in send in server program\n";
}


/*------------------------------------*/
/* function	   drop packet            */
/*------------------------------------*/

bool dropPacket() 
{
	if((rand() % 100) < rate)
		return true;

	else
		return false;
}

/*------------------------------------*/
/* function	   clear buffer           */
/*------------------------------------*/
void clearbuf(char* buf, int size) 
{
	for (int i=0; i<size; i++) 
		buf[i] = 0;
	
}
  
	

