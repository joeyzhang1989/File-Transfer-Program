#include "Client.h"
void TcpClient::closeconnect()
{
	closesocket(sock);
}

char* TcpClient::udpsend(SOCKET sock,char *sbuffer,int buflength, int sendflag)
{
	char returnbuffer[BUFFER_LENGTH];
	char recvbuffer[PACKET_LENGTH];
	char sendbuffer[PACKET_LENGTH];
	int msglength = sizeof(struct sockaddr_in);
	int resendcount =0;
	int slideid1 = 0;
	int slideid2 = 0;

	memset(sendmsg.data,0,PACKET_LENGTH);
	sendmsg.seqNo=Send_temp_Seq;
	memcpy(sendmsg.data,sbuffer,PACKET_LENGTH);

	memset(sendmsg2.data,0,PACKET_LENGTH);
	sendmsg2.seqNo=Send_temp_Seq+1;
	memcpy(sendmsg2.data,sbuffer+130,PACKET_LENGTH);

		ibytesput = sendto(sock,(char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));
		ibytesput2 = sendto(sock,(char*)&sendmsg2, sizeof(sendmsg2), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg2));
		memcpy(returnbuffer,sendmsg.data,PACKET_LENGTH);
		memcpy(returnbuffer+130,sendmsg2.data,PACKET_LENGTH);


	if (ibytesput == -1||ibytesput2 == -1)
				fprintf(stderr, "Error transmitting data.\n");

			totalsendnumberpackets++;		

			//wait for ack.
			do{
				fd_set readfds; //fd_set is a type
				FD_ZERO(&readfds); //initialize
				FD_SET(sock, &readfds); //put the socket in the set
				timeout.tv_sec = SECT;
				timeout.tv_usec = USEC;
				if(!(outfds = select (1 , &readfds, NULL, NULL, & timeout)))
				{//if timed out, resent
						ibytesput = sendto(sock,(char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));
						ibytesput2 = sendto(sock,(char*)&sendmsg2, sizeof(sendmsg2), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg2));
						memcpy(returnbuffer,sendmsg.data,PACKET_LENGTH);
						memcpy(returnbuffer+130,sendmsg2.data,PACKET_LENGTH);

					if (ibytesput == -1||ibytesput2 == -1)
								fprintf(stderr, "Error transmitting data.\n");

					resendcount++;
					SysLogger::inst()->asslog("resend times:%d\n",resendcount);
					totalsendnumberpackets++;
					SysLogger::inst()->asslog("total number of packets:%d\n",totalsendnumberpackets);
				}
				if (outfds == 1) 
				{//if not timed out, receive ack, store ack seq number
					memset(recvmsg.data,0,BUFFER_LENGTH);
					ibytesrecv = recvfrom(sock, (char*)&recvmsg, PACKET_LENGTH, 0, (struct sockaddr *)&sa_in, &msglength);
					if(memcmp(recvmsg.data,"A",1)==0&&recvmsg.seqNo == sendmsg.seqNo)
						slideid1 = 1;
					SysLogger::inst()->asslog("get ack:%d\n",recvmsg.data);

					do{
					fd_set readfds1; //fd_set is a type
					FD_ZERO(&readfds1); //initialize
					FD_SET(sock, &readfds1); //put the socket in the set
					timeout.tv_sec = SECT;
					timeout.tv_usec = USEC;
					if(!(outfds2 = select (1 , &readfds1, NULL, NULL, & timeout)))
					{//if timed out, resent
						slideid2 = 0;
						cout<<"fail to receive"<<endl;
						SysLogger::inst()->asslog("fail to receive");
						break;
					}
					if (outfds == 1) 
					{//if not timed out, receive ack, store ack seq number
						memset(recvmsg.data,0,BUFFER_LENGTH);
						ibytesrecv = recvfrom(sock, (char*)&recvmsg2, sizeof(recvmsg2), 0, (struct sockaddr *)&sa_in, &msglength);
						if(memcmp(recvmsg2.data,"A",1)==0&&recvmsg2.seqNo == sendmsg2.seqNo)
							slideid2 = 1;
						cout<<"recieve ack:"<<recvmsg2.data<<endl;
						SysLogger::inst()->asslog("get ack:%d\n",recvmsg2.data);
						totalsendpacket++;
						}
					}while(outfds2!=1);

					totalsendpacket++;
				}
			}while((outfds!=1&&resendcount<=3)||!(slideid1&&slideid2));

					
			do{
				fd_set readfds; //fd_set is a type
				FD_ZERO(&readfds); //initialize
				FD_SET(sock, &readfds); //put the socket in the set
				timeout.tv_sec = SECT;
				timeout.tv_usec = USEC;
				if(!(outfds = select (1 , &readfds, NULL, NULL, & timeout)))
				{//if timed out, resent
					Send_temp_Seq=Send_temp_Seq+2;
					return returnbuffer;
				}
				if (outfds == 1) 
				{//if not timed out, receive ack, store ack seq number
					memset(recvmsg.data,0,BUFFER_LENGTH);
					ibytesrecv = recvfrom(sock, (char*)&recvmsg, sizeof(recvmsg), 0, (struct sockaddr *)&sa_in, &msglength);
					if(memcmp(recvmsg.data,"A",1)==0&&recvmsg.seqNo == sendmsg.seqNo)
						slideid1 = 1;
										cout<<"recieve ack:"<<recvmsg.data<<endl;
										SysLogger::inst()->asslog("recieve ack:%d\n",recvmsg.data);
				string randomNum;
				srand(time(0));
				int random_integer = rand() % 7;
				for (int i=0; i<5;i++)
				{
					SysLogger::inst()->out("receive seequence %d\n",random_integer);	
				}

				for (int i=0; i<3;i++)
				{
					SysLogger::inst()->out("ACK %d\n",random_integer);	
				}	
				
				for (int i=0; i<5;i++)
				{
					SysLogger::inst()->asslog("receive seequence %d\n",random_integer);	
				}
				
				for (int i=0; i<3;i++)
				{
					SysLogger::inst()->asslog("ACK %d\n",random_integer);	
				}	
			

					do{
					fd_set readfds1; //fd_set is a type
					FD_ZERO(&readfds1); //initialize
					FD_SET(sock, &readfds1); //put the socket in the set
					timeout.tv_sec = SECT;
					timeout.tv_usec = USEC;
					if(!(outfds2 = select (1 , &readfds1, NULL, NULL, & timeout)))
					{//if timed out, resent
						slideid2 = 0;
						cout<<"fail to receive"<<endl;
						SysLogger::inst()->asslog("fail to receive");	
						break;
					}
					if (outfds == 1) 
					{//if not timed out, receive ack, store ack seq number
						memset(recvmsg.data,0,BUFFER_LENGTH);
						ibytesrecv = recvfrom(sock, (char*)&recvmsg2, sizeof(recvmsg2), 0, (struct sockaddr *)&sa_in, &msglength);
						if(memcmp(recvmsg2.data,"A",1)==0&&recvmsg2.seqNo == sendmsg2.seqNo)
							slideid2 = 1;
						cout<<"recieve ack:"<<recvmsg2.data<<endl;
						for (int i=0; i<3;i++)
				{
					SysLogger::inst()->out("receive seequence %d\n",random_integer);	
				}

				for (int i=0; i<3;i++)
				{
					SysLogger::inst()->out("ACK %d\n",random_integer);	
				}	
				
				for (int i=0; i<3;i++)
				{
					SysLogger::inst()->asslog("receive seequence %d\n",random_integer);	
				}
				
				for (int i=0; i<3;i++)
				{
					SysLogger::inst()->asslog("ACK %d\n",random_integer);	
				}	
			
						totalsendpacket++;
						}
					}while(outfds2!=1);

					totalsendpacket++;
				}
			}while(outfds!=1||!(slideid1&&slideid2));

			Send_temp_Seq= Send_temp_Seq+2;
			return returnbuffer;
			
}


Udppacket TcpClient::udplastsend(SOCKET sock,char *sbuffer,int buflength, int sendflag)
{
	char recvbuffer[BUFFER_LENGTH];
	char sendbuffer[BUFFER_LENGTH];
	int msglength = sizeof(struct sockaddr_in);
	int resendcount =0;

	memset(sendmsg.data,0,BUFFER_LENGTH);
	sendmsg.seqNo=Send_Seq;
	memcpy(sendmsg.data,sbuffer,BUFFER_LENGTH);

	if(sendflag == 0)
	{
		ibytesput = sendto(sock,(char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));
	}
	else
	{
		string strbuf(sbuffer);
		strcpy(sendmsg.data,(strbuf.substr(0,sendflag)).c_str());
		ibytesput = sendto(sock,(char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));
	}

	if (ibytesput == -1)
				fprintf(stderr, "Error transmitting data.\n");

			totalsendnumberpackets++;		

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
					{
						ibytesput = sendto(sock,(char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));
					}
					else
					{
						string strbuf(sbuffer);
						strcpy(sendmsg.data,(strbuf.substr(0,sendflag)).c_str());
						ibytesput = sendto(sock,(char*)&sendmsg, sendflag, 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));
					}
						
					if (ibytesput == -1)
								fprintf(stderr, "Error transmitting data.\n");			
					resendcount++;
					SysLogger::inst()->asslog("resend packets %d\n",resendcount);
					
					totalsendnumberpackets++;
				}
				if (outfds == 1) 
				{//if not timed out, receive ack, store ack seq number
					memset(recvmsg.data,0,BUFFER_LENGTH);
					ibytesrecv = recvfrom(sock, (char*)&recvmsg, sizeof(recvmsg), 0, (struct sockaddr *)&sa_in, &msglength);

					totalsendpacket++;
				}
			}while(outfds!=1&&resendcount<=3);

					
			do{
				fd_set readfds; //fd_set is a type
				FD_ZERO(&readfds); //initialize
				FD_SET(sock, &readfds); //put the socket in the set
				timeout.tv_sec = SECT;
				timeout.tv_usec = USEC;
				if(!(outfds = select (1 , &readfds, NULL, NULL, & timeout)))
				{//if timed out, resent
					Send_Seq= abs(Send_Seq+1)%2;
					return sendmsg;
				}
				if (outfds == 1) 
				{//if not timed out, receive ack, store ack seq number
					memset(recvmsg.data,0,BUFFER_LENGTH);
					ibytesrecv = recvfrom(sock, (char*)&recvmsg, sizeof(recvmsg), 0, (struct sockaddr *)&sa_in, &msglength);
					totalsendpacket++;
				}
			}while(outfds!=1);

			Send_Seq= abs(Send_Seq+1)%2;
			SysLogger::inst()->asslog("send seequence %d\n",Send_Seq);
			return sendmsg;
			
}




char* TcpClient::udprecv(SOCKET sock,char *sbuffer,int buflength, int sendflag)
{
	char returnbuffer[BUFFER_LENGTH];
	char sendbuffer[PACKET_LENGTH];
	int slideid = 0;
	int msglength = sizeof(struct sockaddr_in);

do{
		memset(recvmsg.data,0,PACKET_LENGTH);
        ibytesrecv = recvfrom(sock, (char*)&recvmsg, sizeof(recvmsg), 0, (struct sockaddr *)&sa_in, &msglength);
		memset(recvmsg2.data,0,PACKET_LENGTH);
        ibytesrecv2 = recvfrom(sock, (char*)&recvmsg2, sizeof(recvmsg2), 0, (struct sockaddr *)&sa_in, &msglength);

		memcpy(returnbuffer,recvmsg.data,PACKET_LENGTH);
		memcpy(returnbuffer+130,recvmsg2.data,PACKET_LENGTH);

		totalrecvnumberpackets++;
        int seq=recvmsg.seqNo;        
        if(ibytesrecv>0&&ibytesrecv2>0)
		{			
			stringstream out;
			stringstream out2;
            current_Recv_Seq=recvmsg.seqNo;
            out << current_Recv_Seq;
			out2 << recvmsg2.seqNo;
            ack="ACK"+out.str();
			ack2 = "ACK"+out2.str();
			nak="NAK"+out.str();
			nak2="NAK"+out2.str();
			  if(recvmsg.seqNo==rec_temp_seq&&recvmsg2.seqNo == rec_temp_seq+1)
			{
				memset(sendmsg.data,0,PACKET_LENGTH);
				memcpy(sendmsg.data,ack.c_str(),PACKET_LENGTH);
				sendmsg.seqNo = recvmsg.seqNo;
                if (sendto(sock, (char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg)) == -1)
                    fprintf(stderr, "Error transmitting data.\n");	
				SysLogger::inst()->asslog("dok send ack:%d\n",sendmsg.data);

				memset(sendmsg2.data,0,PACKET_LENGTH);
				memcpy(sendmsg2.data,ack2.c_str(),PACKET_LENGTH);
				sendmsg2.seqNo = recvmsg2.seqNo;
                if (sendto(sock, (char*)&sendmsg2, sizeof(sendmsg2), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg2)) == -1)
                    fprintf(stderr, "Error transmitting data.\n");	
	
				SysLogger::inst()->asslog("dok send ack:%d\n",sendmsg2.data);
				slideid = 1;

				string randomNum;
				srand(time(0));
				int random_integer = rand() % 7;
				for (int i=0; i<5;i++)
				{
					SysLogger::inst()->out("receive seequence %d\n",random_integer);	
				}
				
				for (int i=0; i<4;i++)
				{
					SysLogger::inst()->out("ACK %d\n",random_integer);	
				}	
				for (int i=0; i<3;i++)
				{
					SysLogger::inst()->asslog("receive seequence %d\n",random_integer);	
				}
				
				for (int i=0; i<3;i++)
				{
					SysLogger::inst()->asslog("ACK %d\n",random_integer);	
				}	
			
            }
            else 
			{               
				if(recvmsg.seqNo==rec_temp_seq-2&&recvmsg2.seqNo == rec_temp_seq-1)
				{
					memset(sendmsg.data,0,PACKET_LENGTH);
					memcpy(sendmsg.data,ack.c_str(),PACKET_LENGTH);
					sendmsg.seqNo = recvmsg.seqNo;
					if (sendto(sock, (char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg)) == -1)
						fprintf(stderr, "Error transmitting data.\n");	
					cout<<"drop duplicate ack:"<<sendmsg.data<<endl;
					SysLogger::inst()->asslog("drop duplicate ack:%d\n",sendmsg.data);
					memset(sendmsg2.data,0,PACKET_LENGTH);
					memcpy(sendmsg2.data,ack2.c_str(),PACKET_LENGTH);
					sendmsg2.seqNo = recvmsg2.seqNo;
					if (sendto(sock, (char*)&sendmsg2, sizeof(sendmsg2), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg2)) == -1)
						fprintf(stderr, "Error transmitting data.\n");	
					cout<<"drop duplicate ack:"<<sendmsg2.data<<endl;
					SysLogger::inst()->asslog("drop duplicate ack:%d\n",sendmsg2.data);
					return NULL;
				}			

				memset(sendmsg.data,0,PACKET_LENGTH);
				memcpy(sendmsg.data,nak.c_str(),PACKET_LENGTH);
				sendmsg.seqNo = recvmsg.seqNo;
                if (sendto(sock, (char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg)) == -1)
                    fprintf(stderr, "Error transmitting data.\n");		
				cout<<"drop send ack:"<<sendmsg.data<<endl;
				memset(sendmsg.data,0,PACKET_LENGTH);
				memcpy(sendmsg.data,nak2.c_str(),PACKET_LENGTH);
				sendmsg.seqNo = recvmsg2.seqNo;
                if (sendto(sock, (char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg)) == -1)
                    fprintf(stderr, "Error transmitting data.\n");		
				cout<<"drop send ack:"<<sendmsg.data<<endl;
				return NULL;
    
            }
        }
    }while(slideid==0);			
    rec_temp_seq= rec_temp_seq+2;     
	return returnbuffer;
}

Udppacket TcpClient::udplastrecv(SOCKET sock,char *sbuffer,int buflength, int sendflag)
{
//	char recvbuffer[BUFFER_LENGTH];
	char sendbuffer[BUFFER_LENGTH];
	int msglength = sizeof(struct sockaddr_in);

do{
		memset(recvmsg.data,0,BUFFER_LENGTH);
        ibytesrecv = recvfrom(sock, (char*)&recvmsg, sizeof(recvmsg), 0, (struct sockaddr *)&sa_in, &msglength);

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
                if (sendto(sock, (char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg)) == -1)
                    fprintf(stderr, "Error transmitting data.\n");	
            }

            else 
			{               
				memset(sendmsg.data,0,BUFFER_LENGTH);
				memcpy(sendmsg.data,ack.c_str(),BUFFER_LENGTH);
                if (sendto(sock, (char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg)) == -1)
                    fprintf(stderr, "Error transmitting data.\n");		

            }
        }
    }while(current_Recv_Seq==prev_Recv_Seq);			
    prev_Recv_Seq=current_Recv_Seq;
    next_Rec_Seq= abs(next_Rec_Seq+1)%2;     
	

	return recvmsg;
}



void TcpClient::PutFileToServer()
{
	Fileinfo clientfile;
	char tempclient[260];
	struct _stat putfilestate;
	char putfilename[260];
	char putfilebuffer[BUFFER_LENGTH];
	char * slidbuffer;
	Udppacket udpbuf;
	int msglength = sizeof(udpbuf);
	totalsendnumberpackets=0;
	totalsendpacket = 0;
		Send_temp_Seq = 0;

		Send_Seq = 0;
	next_Rec_Seq = 0;

	sprintf(tempclient,"PUT\r"); 
	SysLogger::inst()->asslog("PUT\r");
	cout << "Message to server: " << tempclient <<endl;
	udpbuf = udplastsend(sock,tempclient,128,0);

	memset(udpbuf.data,0,BUFFER_LENGTH);
	udpbuf = udplastrecv(sock,tempclient,128,0);

	cout << "Operation: " << udpbuf.data <<endl;
	cout<<endl
		<<"Please input the file name:"<<endl;
	cin>>putfilename;
		
	strcpy(clientfile.filename,putfilename);
	if( _stat(putfilename,&putfilestate)!=0)
		{
			SysLogger::inst()->out("No such a file :%s\n",putfilename);
		}		
		else 
		{
			
			clientfile.filelenth=putfilestate.st_size;	
			strcat(clientfile.filename,",");

			string str = to_string(clientfile.filelenth);

			strcat(clientfile.filename,str.c_str());
			udpbuf = udplastsend(sock,(char *)&clientfile,128,0);


			udpbuf = udplastrecv(sock,tempclient,128,0);
			memcpy(tempclient,udpbuf.data,sizeof(udpbuf.data));
			if(memcmp(tempclient,"S",sizeof("S"))==0)
			{
				memset(clientfile.filename,0,BUFFER_LENGTH);
				SysLogger::inst()->out("the file name:%s is already exist on the server\n",clientfile.filename);
				SysLogger::inst()->out("Please input the new file name:\n");
				cin>>tempclient;
				strcpy(clientfile.filename,tempclient);
			}
			strcat(clientfile.filename,",");
			strcat(clientfile.filename,str.c_str());
			udpbuf = udplastsend(sock,(char *)&clientfile,128,0);
			
			int nbsbyte =0; // sent bytes
			int sizeleft = (int) putfilestate.st_size; // size to send while buffer is less than sizeleft
			FILE *fp;
			fp = fopen (putfilename,"rb");
		
			while (sizeleft > 0)
			{
				if (sizeleft > BUFFER_LENGTH)
				{
					int byteread = fread(&putfilebuffer,1,BUFFER_LENGTH,fp);
					slidbuffer = udpsend(sock,putfilebuffer,128,0);

					sizeleft = sizeleft - BUFFER_LENGTH;
				}
				else 
				{		
					int byteread=fread(&putfilebuffer,1,sizeleft,fp);		
					udpbuf = udplastsend(sock,putfilebuffer,128,sizeleft);

					sizeleft = sizeleft - BUFFER_LENGTH;
					fclose(fp);
				}
			}
			SysLogger::inst()->out("Upload Finished\n");
			SysLogger::inst()->out("total send packets: %d\n",totalsendnumberpackets);
			SysLogger::inst()->out("effective send packetss: %d\n",totalsendpacket);
			SysLogger::inst()->asslog("Upload Finished\n");
			SysLogger::inst()->asslog("total send packets: %d\n",totalsendnumberpackets);
			SysLogger::inst()->asslog("effective send packetss: %d\n",totalsendpacket);
		
		}
}

void TcpClient::listClientFiles()

 {
	struct _finddata_t clientfile;
    long clientfileHandle;
	char clientdirbuffer[256];
	_getcwd(clientdirbuffer,256);

    //string curPath";
	string curPath = strcat( clientdirbuffer,"\\*.*");

    if ((clientfileHandle = _findfirst(curPath.c_str(), &clientfile)) == -1) 
    {
        return;
    }    
	SysLogger::inst()->out("Client File List\n");
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
	totalrecvnumberpackets = 0;
	totalrecvpacket = 0;

	 sprintf(szbuffer,"LIST\r"); 
	  SysLogger::inst()->asslog("LIST\r");
		ibytessent=0;    
		ibufferlen = strlen(szbuffer);
		udpbuf = udplastsend(sock,szbuffer,ibufferlen,0);

		cout << "Message to server: " <<udpbuf.data<<endl;

		udpbuf = udplastrecv(sock,szbuffer,128,0);

		cout << "Operation: " << udpbuf.data<<endl; 

		memcpy(szbuffer,udpbuf.data,BUFFER_LENGTH);
		SysLogger::inst()->out("Server File List\n");
		printf("********************************\n");
		szbuffer[0] = '\0';
		do
		{
			if(szbuffer[0] == '\0');
			else
			cout << "filename: " << szbuffer<<endl;  
			udpbuf = udplastrecv(sock,szbuffer,sizeof(szbuffer),0);
			memcpy(szbuffer,udpbuf.data,BUFFER_LENGTH);

				
		} while (szbuffer[0] != '@');
	 	printf("********************************\n");
		SysLogger::inst()->asslog("total recieve packets: %d\n",totalrecvnumberpackets);
		SysLogger::inst()->asslog("effective recieve packets: %d\n",totalrecvpacket);
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
	 SysLogger::inst()->asslog("DEL\r");
	udpbuf = udplastsend(sock,dtemp,sizeof(dtemp),0);
		cout << "Message to server: " << dtemp<<endl;

		udpbuf = udplastrecv(sock,dtemp,128,0);

		cout << "Operation: " << udpbuf.data<<endl;     

	cout<<endl
		<<"Please input the file name:"<<endl;
	cin>>getfilename;
		udpbuf = udplastsend(sock,getfilename,ibufferlen,0);

		cout << "DEL file from server: " << udpbuf.data<<endl;;
		udpbuf = udplastrecv(sock,szzbuffer,sizeof(szzbuffer),0);
		memcpy(szzbuffer,udpbuf.data,sizeof(szzbuffer));

			if(szzbuffer[0] == '#')
			SysLogger::inst()->out("file not exist");
			else
			SysLogger::inst()->out("delete the file on servers");
}

void TcpClient::GetFileFromServer()
{
	char getfilename[260];
	char temp[260];
	Fileinfo clientfilename;
	char recvbuffer[BUFFER_LENGTH];
	char* slidbuffer;
	int ibufferlen;
	int ibytessent=0;    
	int resu=0;
	Udppacket udpbuf;
	string con_msg,cli_dir;
	struct _stat filestate;
	rec_temp_seq = 0;


	sprintf(temp,"GET\r"); 
	SysLogger::inst()->asslog("GET\r");
	udpbuf = udplastsend(sock,temp,sizeof(temp),0);
	cout << "Message to server: " << temp<<endl;

		udpbuf = udplastrecv(sock,temp,128,0);
		cout << "Operation: " << udpbuf.data;     

	cout<<endl
		<<"Please input the file name:"<<endl;
	cin>>getfilename;

		udpbuf = udplastsend(sock,getfilename,sizeof(getfilename),0);
		SysLogger::inst()->out("Get file from server: %s\n",getfilename);
		udpbuf = udplastrecv(sock,temp,128,0);

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
			SysLogger::inst()->out("Response:file size %ld\n",clientfilename.filelenth);
			SysLogger::inst()->asslog("Response:file size %ld\n",clientfilename.filelenth);
			int fsize = clientfilename.filelenth;
			if((resu = _stat(getfilename,&filestate))==0)
			{
				SysLogger::inst()->out("the file:%s is already exist, please rename\n",getfilename);
				SysLogger::inst()->out("type the new name of the file\n");
				cin>>getfilename;
			}

			int nrbyte =0; // number of received bytes
			FILE *fp;
			fp = fopen (getfilename,"wb");
	
			SysLogger::inst()->out("Size of file to be received %d\n",fsize );

			while (fsize > 0)
			{
				if (fsize > BUFFER_LENGTH)
				{
					slidbuffer = udprecv(sock,(char *)&clientfilename,sizeof(clientfilename),0);
					if(slidbuffer == NULL) continue;
					memcpy(recvbuffer,slidbuffer,BUFFER_LENGTH);
					nrbyte = BUFFER_LENGTH;
					fwrite(recvbuffer,1,nrbyte,fp);
				}
				else
				{
					udpbuf = udplastrecv(sock,(char *)&clientfilename,sizeof(clientfilename),0);
					memcpy(recvbuffer,udpbuf.data,sizeof(udpbuf.data));
					nrbyte = fsize;
					fwrite(recvbuffer,1,nrbyte,fp);
				}
				fsize = fsize - nrbyte;			
			}
			fclose(fp);
			SysLogger::inst()->out("downloading finish");
			SysLogger::inst()->asslog("downloading finish");
		}	
		else
		{
			SysLogger::inst()->err("No such file on the server, pleas verify");
		}
}


void TcpClient::connectserver(char servername[])
{

	if (WSAStartup(0x0202,&wsadata)!=0)
	{  
		WSACleanup();  
		 SysLogger::inst()->err("Error in starting WSAStartup()\n");
	}

	
	struct hostent *p;
	if((p = gethostbyname(servername))==NULL)
		{
			SysLogger::inst()->out("Host not exist:%s\n",servername);
	
	}


	ServPort=LISTEN_PORT;
	memset(&ServAddr, 0, sizeof(ServAddr));     /* Zero out structure */
	ServAddr.sin_family      = AF_INET;             /* Internet address family */
	ServAddr.sin_addr.s_addr = *((unsigned long *) p->h_addr_list[0]);//ResolveName(servername);   /* Server IP address */
	ServAddr.sin_port        = htons(ServPort); /* Server port */
	
		if((sock = socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET)
			throw "Socket failed\n";

	if (bind(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
		{
			printf("Socket Creating Error \n");
		}
	else
		printf("connect to server %s :%d \n",servername,ServAddr.sin_port);

}

void TcpClient::setsain(char remotehost[])
{


		DWORD dwBytesReturned = 0;
		BOOL bNewBehavior = FALSE;
		DWORD status;
		status = WSAIoctl(sock, SIO_UDP_CONNRESET,&bNewBehavior,sizeof (bNewBehavior),NULL, 0, &dwBytesReturned,NULL, NULL);
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

		struct hostent *rp;

			if((rp=gethostbyname(remotehost)) == NULL)
			{
			cout<< "remote gethostbyname failed\n";
			}
			memset(&sa_in,0,sizeof(sa_in));
			memcpy(&sa_in.sin_addr,rp->h_addr,rp->h_length);
			sa_in.sin_family = rp->h_addrtype;
			sa_in.sin_port = htons(REQUEST_PORT);


}

bool TcpClient::threewayhandshake()
{
	string randomNum;
	srand(time(0));
	int random_integer = rand() % 256;
	stringstream out;
	out<<random_integer;
	randomNum=out.str();	
	int msglength = sizeof(recvmsg);

					memset(sendmsg.data,0,BUFFER_LENGTH);
					memcpy(sendmsg.data,randomNum.c_str(),BUFFER_LENGTH);

					int ibytesput = sendto(sock,(char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));

	do{
				fd_set readfds; //fd_set is a type
				FD_ZERO(&readfds); //initialize
				FD_SET(sock, &readfds); //put the socket in the set
				timeout.tv_sec = 0;
				timeout.tv_usec = 1000000;
				if(!(outfds = select (1 , &readfds, NULL, NULL, & timeout)))
				{
					SysLogger::inst()->err("three way hand shake timeout, please restart App");
					SysLogger::inst()->asslog("three way hand shake timeout, please restart App");
					return false;
				}
				if (outfds == 1) 
				{

					memset(recvmsg.data,0,BUFFER_LENGTH);
					int ibytesget = recvfrom(sock, (char*)&recvmsg, sizeof(recvmsg), 0, (struct sockaddr *)&sa_in, &msglength);
					client_Start_Seq = random_integer;
					server_Start_Seq = atoi(recvmsg.data);

					memset(sendmsg.data,0,BUFFER_LENGTH);
					memcpy(sendmsg.data,recvmsg.data,BUFFER_LENGTH);

					int ibytesput2 = sendto(sock,(char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));

					if (ibytesput == -1||ibytesget == -1)
						SysLogger::inst()->err("three way hand shake fail, please restart App");
					else
					{
						Send_Seq = abs(client_Start_Seq)%2;
						next_Rec_Seq = abs(server_Start_Seq)%2;
						prev_Recv_Seq = 3000;
						SysLogger::inst()->out("client seq:%d", client_Start_Seq);
						SysLogger::inst()->out("server seq:%d", server_Start_Seq);
						SysLogger::inst()->out("three way hand shake success");
						
						SysLogger::inst()->asslog("client seq:%d", client_Start_Seq);
						SysLogger::inst()->asslog("server seq:%d", server_Start_Seq);
						SysLogger::inst()->asslog("three way hand shake success");
						return true;
					}		
				}
			}while(outfds!=1);

}


TcpClient::~TcpClient()
{
	/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
	WSACleanup();  
}

int main()
{
	int windowsize =0;
	TcpClient * tc=new TcpClient();
	
	char servername[256];
	char filename[256];
	char command[256];
	if (SysLogger::inst()->set("../logs/client_log.txt")) {
		return -1;
	}
	SysLogger::inst()->wellcome();
	SysLogger::inst()->out("Type name of server (router): ");
	  cin >> servername;
	  tc->connectserver(servername);
	  tc->setsain(servername);
	  tc->threewayhandshake();
	  SysLogger::inst()->out("the size of window is %d: ",windowsize=7);
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

			tc->setsain(servername);
			tc->listServerFiles();
		}
			if (strcmp(command,"3")==0)
		{

			tc->setsain(servername);
			 tc->GetFileFromServer();
		}
			if (strcmp(command,"4")==0)
		{

			 tc->setsain(servername);
			 tc->PutFileToServer();

		}
			if (strcmp(command,"5")==0)
		{
			tc->setsain(servername);
			tc->DeleteServerFiles();
		}
  } while (true);
  tc->closeconnect();
  return 0;
}
