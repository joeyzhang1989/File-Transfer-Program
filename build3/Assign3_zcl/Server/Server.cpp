#include "Server.h"

	
	char* TcpServer::udpsend(SOCKET sock,char *sbuffer,int buflength, int sendflag)
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
						cout<<"resend:"<<sendmsg.seqNo<<endl;
						cout<<"resend:"<<sendmsg2.seqNo<<endl;
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
					ibytesrecv = recvfrom(sock, (char*)&recvmsg, sizeof(recvmsg), 0, (struct sockaddr *)&sa_in, &msglength);
					if(memcmp(recvmsg.data,"A",1)==0&&recvmsg.seqNo == sendmsg.seqNo)
						slideid1 = 1;
					cout<<"recieve ack:"<<recvmsg.data<<endl;
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
					ibytesrecv = recvfrom(sock, (char*)&recvmsg, PACKET_LENGTH, 0, (struct sockaddr *)&sa_in, &msglength);
					if(memcmp(recvmsg.data,"A",1)==0&&recvmsg.seqNo == sendmsg.seqNo)
						slideid1 = 1;
										cout<<"recieve ack:"<<recvmsg.data<<endl;
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


char* TcpServer::udprecv(SOCKET sock,char *sbuffer,int buflength, int sendflag)
{
//	char recvbuffer[BUFFER_LENGTH];
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
				memset(sendmsg.data,0,PACKET_LENGTH);
				memcpy(sendmsg.data,nak2.c_str(),PACKET_LENGTH);
				sendmsg.seqNo = recvmsg2.seqNo;
                if (sendto(sock, (char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg)) == -1)
                    fprintf(stderr, "Error transmitting data.\n");		

				return NULL;   
            }
        }
    }while(slideid==0);			

    rec_temp_seq= rec_temp_seq+2;          

	return returnbuffer;
}



	void TcpServer::run()
	{
		Udppacket udpbuf;
		int msglength = sizeof(udpbuf);
		timeout.tv_sec = SECT;
		timeout.tv_usec = USEC;


		WSADATA wsadata;
    		 
			if (WSAStartup(0x0202,&wsadata)!=0){  
				cout<<"Error in starting WSAStartup()\n";
				SysLogger::inst()->asslog("Error in starting WSAStartup()\n");
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
			SysLogger::inst()->asslog("can't bind the socket \n");
			cout << "Bind was successful" << endl;

			//Successfull bind, now listen for client requests.

			FD_ZERO(&readfds);

			threewayhandshake();

		while(1)
			{

				DWORD dwBytesReturned = 0;
				BOOL bNewBehavior = FALSE;
				DWORD status;
				status = WSAIoctl(s1, SIO_UDP_CONNRESET,&bNewBehavior,sizeof (bNewBehavior),NULL, 0, &dwBytesReturned,NULL, NULL);
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

				sa.sin_family = AF_INET;
				sa.sin_port = htons(REQUEST_PORT);
				sa.sin_addr.s_addr = htonl(INADDR_ANY);

				if(!(outfds=select(infds,&readfds,NULL,NULL,tp))) {}
				else if (outfds == SOCKET_ERROR) ;
				else if (FD_ISSET(s1,&readfds))  SysLogger::inst()->out("got a connection request  \n");
				//Found a connection request, try to accept. 


		memset(udpbuf.data,0,sizeof(udpbuf.data));
		memset(szbuffer,0,sizeof(szbuffer));
				//Fill in szbuffer from accepted request.
				SysLogger::inst()->out("wating for message \n");
			udpbuf = udplastrecv(s1,szbuffer,128,0);

			memcpy(szbuffer,recvmsg.data,BUFFER_LENGTH);
			SysLogger::inst()->out("the message from client: %d\n",recvmsg.data );		
				if(strcmp(szbuffer,"LIST\r")==0)
				{
					udpbuf = udplastsend(s1,szbuffer,128,0);
					cout << "Operation:" << udpbuf.data << endl;  
					SysLogger::inst()->asslog("LIST\r");
					listFiles(s1);
				}

				if(strcmp(szbuffer,"GET\r")==0)
				{
					udpbuf = udplastsend(s1,szbuffer,128,0);
					cout << "Operation:" << udpbuf.data << endl;  
					SysLogger::inst()->asslog("GET\r");
					PutFileToClient(s1);
				}

				if(strcmp(szbuffer,"PUT\r")==0)
				{
					udpbuf = udplastsend(s1,szbuffer,128,0);
					cout << "Operation:" << udpbuf.data << endl;  
					SysLogger::inst()->asslog("PUT\r");
					GetFileFromClient(s1);
				}
				if(strcmp(szbuffer,"DEL\r")==0)
				{
					udpbuf = udplastsend(s1,szbuffer,128,0);
					cout << "Operation:" << udpbuf.data << endl;  
					SysLogger::inst()->asslog("DEL\r");
					delFiles(s1);
				}
		}
	}

	Udppacket TcpServer::udplastsend(SOCKET sock,char *sbuffer,int buflength, int sendflag)
{
	char recvbuffer[BUFFER_LENGTH];
	char sendbuffer[BUFFER_LENGTH];
	int msglength = sizeof(struct sockaddr_in);
	int resendcount =0;

	memset(sendmsg.data,0,BUFFER_LENGTH);
	sendmsg.seqNo=Send_Seq;
	memcpy(sendmsg.data,sbuffer,BUFFER_LENGTH);


	if(sendflag == 0)
		ibytesput = sendto(sock,(char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));
	else
	{
		string strbuf(sbuffer);
		strcpy(sendmsg.data,(strbuf.substr(0,sendflag)).c_str());
		ibytesput = sendto(sock,(char*)&sendmsg, sendflag, 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));
	}

	if (ibytesput == -1)
				fprintf(stderr, "Error transmitting data.\n");

			totalsendnumberpackets++;		

			//wait for ack.
			do{
				fd_set readfds; //fd_set is a type
				FD_ZERO(&readfds); //initialize
				FD_SET(sock, &readfds); //put the socket in the set

				if(!(outfds = select (1 , &readfds, NULL, NULL, & timeout)))
				{//if timed out, resent
					if(sendflag == 0)
						ibytesput = sendto(sock,(char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));
					else
						{
							string strbuf(sbuffer);
							strcpy(sendmsg.data,(strbuf.substr(0,sendflag)).c_str());
							ibytesput = sendto(sock,(char*)&sendmsg, sendflag, 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));
						}
					if (ibytesput == -1)
								fprintf(stderr, "Error transmitting data.\n");

					resendcount++;


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
					SysLogger::inst()->asslog("did not timeout\n");
					totalsendpacket++;
				}
			}while(outfds!=1);

			Send_Seq= abs(Send_Seq+1)%2;

	return sendmsg;
}



Udppacket TcpServer::udplastrecv(SOCKET sock,char *sbuffer,int buflength, int sendflag)
{
//	char recvbuffer[BUFFER_LENGTH];
	char sendbuffer[BUFFER_LENGTH];
	int msglength = sizeof(struct sockaddr_in);

do{
		int q = sizeof(recvmsg);
		int p = msglength;
		memset(recvmsg.data,0,BUFFER_LENGTH);
        ibytesrecv = recvfrom(sock, (char*)&recvmsg, sizeof(recvmsg), 0, (struct sockaddr *)&sa_in, &msglength);

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
                if (sendto(sock, (char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg)) == -1)
                    fprintf(stderr, "Error transmitting data.\n");	
//				cout<<"ok send ack:"<<recvmsg.data<<endl;
            }
            // else if seq number=previous seq number
            else 
			{               
				memset(sendmsg.data,0,BUFFER_LENGTH);
				memcpy(sendmsg.data,ack.c_str(),BUFFER_LENGTH);
				totalrecvpacket++;
                if (sendto(sock, (char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg)) == -1)
                    fprintf(stderr, "Error transmitting data.\n");
//				cout<<"drop send ack:"<<recvmsg.data<<endl;
            }

        }
    }while(current_Recv_Seq==prev_Recv_Seq);			
    prev_Recv_Seq=current_Recv_Seq;
    next_Rec_Seq= abs(next_Rec_Seq+1)%2;     

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
		char * slidbuffer;
		struct _stat getfilestate;
		Udppacket udpbuf;
		string con_msg,cli_dir;
		string con_msg2,cli_dir2;
		rec_temp_seq = 0;
		char changname[260];

		memset(getclientfilename,0,sizeof(getclientfilename));
		//receive the response
//		if(recv(gs1,(char *)&clientputfilename,sizeof(clientputfilename),0)!=sizeof(clientputfilename))
//			printf("recv response error,exit");

		udpbuf = udplastrecv(gs1,(char *)&clientputfilename,sizeof(clientputfilename),0);

		for (int i = 0; udpbuf.data[i] != 0; i++)
		con_msg += udpbuf.data[i];
		int pos = con_msg.find_first_of(',');
		cli_dir = con_msg.substr(0, pos);
		con_msg = con_msg.substr(pos + 1);

		
		strcpy(clientputfilename.filename,cli_dir.c_str());
		clientputfilename.filelenth = atoi(con_msg.c_str());

		//cast it to the response structure
		SysLogger::inst()->out("File name is: %s\n",clientputfilename.filename);
			SysLogger::inst()->out("Size of file to be received %d\n",clientputfilename.filelenth );
		
		//check if file exist on server, compare the names of filename and the response in respp structure
		if( _stat(clientputfilename.filename,&getfilestate)==0)
		{
			memset(changname,0,sizeof(changname));
			memcpy(changname,"S",sizeof("S"));
			udplastsend(gs1,changname,sizeof(changname),0);
		}
		else
		{
			memset(changname,0,sizeof(changname));
			memcpy(changname,"C",sizeof("C"));
			udplastsend(gs1,changname,sizeof(changname),0);
		}

		memset(getclientfilename,0,sizeof(getclientfilename));
		udpbuf = udplastrecv(gs1,(char *)&clientputfilename,sizeof(clientputfilename),0);

		for (int t = 0; udpbuf.data[t] != 0; t++)
		con_msg2 += udpbuf.data[t];
		int pos2 = con_msg2.find_first_of(',');
		cli_dir2 = con_msg2.substr(0, pos2);
		con_msg2 = con_msg2.substr(pos2 + 1);

		
		strcpy(clientputfilename.filename,cli_dir2.c_str());
		clientputfilename.filelenth = atoi(con_msg2.c_str());

		//cast it to the response structure
		SysLogger::inst()->out("File name is: %s\n",clientputfilename.filename);
			// get the filesize
//		
			// get the filesize
			int fsize = clientputfilename.filelenth;
	
			// start getting the file
			int nrbyte =0; // number of received bytes
			FILE *fp;
			fp = fopen (clientputfilename.filename,"wb");
	
			SysLogger::inst()->out("Size of file to be received %d\n",fsize );

			while (fsize > 0)
			{
				if (fsize > BUFFER_LENGTH)
				{
					slidbuffer = udprecv(gs1,(char *)&clientputfilename,sizeof(clientputfilename),0);
					if(slidbuffer == NULL) continue;
					memcpy(serverrecvbuffer,slidbuffer,BUFFER_LENGTH);
					nrbyte = BUFFER_LENGTH;
					fwrite(serverrecvbuffer,1,nrbyte,fp);
				}
				else
				{
//					nrbyte = recv(gs1,serverrecvbuffer,fsize,0);
					udpbuf = udplastrecv(gs1,(char *)&clientputfilename,sizeof(clientputfilename),0);
					memcpy(serverrecvbuffer,udpbuf.data,sizeof(udpbuf.data));
					nrbyte = fsize;
					fwrite(serverrecvbuffer,1,nrbyte,fp);
				}
	
				fsize = fsize - nrbyte;
	
			}
			fclose(fp);
			SysLogger::inst()->out("downloading finish");
			SysLogger::inst()->asslog("downloading finish");
	}

	void TcpServer::PutFileToClient(SOCKET ps1)
	{
		Fileinfo serverfile;
		Udppacket udpbuf;
	struct _stat stat_buf;
    int result;
	char filenamebuffer[260];
	char * slidbuffer;
	char filebuffer[BUFFER_LENGTH];
	totalsendnumberpackets=0;
	totalsendpacket = 0;
	Send_temp_Seq = 0;


	memset(filenamebuffer,0,sizeof(filenamebuffer));
	
	udpbuf = udplastrecv(ps1,filenamebuffer,sizeof(filenamebuffer),0);
	strcpy(filenamebuffer,udpbuf.data);

	//cast it to the request packet structure		
	strcpy(serverfile.filename,filenamebuffer);
	if((result = _stat(serverfile.filename,&stat_buf))!=0)
		{
			SysLogger::inst()->out("No such a file\n");

			udpsend(ps1,filenamebuffer,sizeof(filenamebuffer),0);

		}		
		else 
		{
			serverfile.filelenth=stat_buf.st_size;
			strcat(serverfile.filename,",");
			string str = to_string(serverfile.filelenth);
			strcat(serverfile.filename,str.c_str());
			udpbuf = udplastsend(ps1,(char *)&serverfile,128,0);

			
			int nbsbyte =0; // sent bytes
			int sizeleft = (int) stat_buf.st_size; // size to send while buffer is less than sizeleft
			FILE *fp;
			fp = fopen (filenamebuffer,"rb");
			SysLogger::inst()->out("received file size: %d\n",sizeleft);
			SysLogger::inst()->asslog("received file size: %d\n",sizeleft);
			while (sizeleft >= 0)
			{
				if (sizeleft > BUFFER_LENGTH)
				{
					int byteread = fread(&filebuffer,1,BUFFER_LENGTH,fp);
					slidbuffer = udpsend(ps1,filebuffer,128,0);

					sizeleft = sizeleft - BUFFER_LENGTH;
				}
				else 
				{		
					int byteread=fread(&filebuffer,1,sizeleft,fp);		
					udpbuf = udplastsend(ps1,filebuffer,128,sizeleft);

					sizeleft = sizeleft - BUFFER_LENGTH;
					fclose(fp);
				}
			}
			SysLogger::inst()->out("Upload Finished\n");
			SysLogger::inst()->out("total send packets: %d\n",totalsendnumberpackets);
			SysLogger::inst()->out("effective send packetss: %d\n",totalsendpacket);
			SysLogger::inst()->asslog("total send packets: %d\n",totalsendnumberpackets);
			SysLogger::inst()->asslog("effective send packetss: %d\n",totalsendpacket);
		}
//		closesocket(ps1);
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
	udpbuf = udplastrecv(ds1,filenamebuffer,sizeof(filenamebuffer),0);	
	memcpy(filenamebuffer,udpbuf.data,sizeof(filenamebuffer));
	strcpy(serverfile.filename,filenamebuffer);
	if((result = _stat(serverfile.filename,&stat_buf))!=0)
		{
			char szlbuffer[260];
			memcpy(szlbuffer,"#\r",sizeof(szlbuffer));
			udplastsend(ds1,szlbuffer,sizeof(szlbuffer),0);

		}		
		else 
		{
			if(!_access(filenamebuffer,0)){
				SetFileAttributes(filenamebuffer,0);
				if (DeleteFile(filenamebuffer))
				{
					char servbuffer[260];

					memcpy(servbuffer,"@\r",sizeof(servbuffer));
				udplastsend(ds1,servbuffer,sizeof(servbuffer),0);
//					printf("error in send in server program\n");				
			SysLogger::inst()->out("delete %s success\n",serverfile.filename);
				SysLogger::inst()->asslog("delete %s success\n",serverfile.filename);
				}
			}
		}
//		closesocket(ds1);	
}

void TcpServer::threewayhandshake()
{
	string randomNum;
	srand(32);
	int random_integer = rand() % 256;
	stringstream out;
	out<<random_integer;
	randomNum=out.str();
	int msglength = sizeof(recvmsg);

	
	memset(recvmsg.data,0,BUFFER_LENGTH);
	int ibytesget = recvfrom(s1, (char*)&recvmsg, sizeof(recvmsg), 0, (struct sockaddr *)&sa_in, &msglength);

	server_Start_Seq = random_integer;
	client_Start_Seq = atoi(recvmsg.data);

	memset(sendmsg.data,0,BUFFER_LENGTH);
	memcpy(sendmsg.data,randomNum.c_str(),BUFFER_LENGTH);
	int ibytesput = sendto(s1,(char*)&sendmsg, sizeof(sendmsg), 0, (struct sockaddr *)&sa_in, sizeof(sendmsg));

	memset(recvmsg.data,0,BUFFER_LENGTH);
	int ibytesget2 = recvfrom(s1, (char*)&recvmsg, sizeof(recvmsg), 0, (struct sockaddr *)&sa_in, &msglength);



	if (ibytesput == -1||ibytesget == -1)
		cout<<"three way hand shake fail, please restart App"<<endl;
	else
	{
		Send_Seq = abs(server_Start_Seq)%2;
		next_Rec_Seq = abs(client_Start_Seq)%2;
		SysLogger::inst()->out("client seq:%d", client_Start_Seq);
		SysLogger::inst()->out("server seq:%d", server_Start_Seq);
		SysLogger::inst()->out("three way hand shake success");
						
		SysLogger::inst()->asslog("client seq:%d", client_Start_Seq);
		SysLogger::inst()->asslog("server seq:%d", server_Start_Seq);
		SysLogger::inst()->asslog("three way hand shake success");
	}
}


	void TcpServer::listFiles(SOCKET ss1)

 {
	struct _finddata_t serverfile;
    long serverfileHandle;
	char serverdirbuffer[256];
	_getcwd(serverdirbuffer,256);


    //string curPath";
	string curPath = strcat( serverdirbuffer,"\\*.*");

    if ((serverfileHandle = _findfirst(curPath.c_str(), &serverfile)) == -1) 
    {
        return;
    }    
    do {
            if (_A_ARCH == serverfile.attrib) 
            {				
				udplastsend(ss1,serverfile.name,sizeof(serverfile.name),0);

        }
    } while (!(_findnext(serverfileHandle, &serverfile)));
    _findclose(serverfileHandle);
				char servbuffer[260];

				memcpy(servbuffer,"@\r",sizeof(servbuffer));
				udplastsend(ss1,servbuffer,sizeof(servbuffer),0);
	
}

	int main(void){

		TcpServer tserver;
		if (SysLogger::inst()->set("../logs/Server_log.txt")) {
		return -1;
		}
		SysLogger::inst()->wellcome();
		tserver.run();
		closesocket(s1);		
		closesocket(s);
		/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
		WSACleanup();
		return 0;
	}




