#include "UDP.h"
//----------------------------------------------------
//Construtor
//----------------------------------------------------
UDP::UDP(int sendPort_t,int requestPort_t)
{
	sendPort=sendPort_t;
	requestPort=requestPort_t;
	fout.open("log.txt");
	WSADATA wsadata;
	try 
	{                  
		if (WSAStartup(0x0202,&wsadata)!=0)
			throw "Error in starting WSAStartup()\n";

		HOSTENT *hp;
		gethostname(localhost,MAXHOSTNAMELEN);
		if((hp=gethostbyname(localhost)) == NULL) 
			throw "gethostbyname failed\n";
		cout<<"Server starting on host:"<<localhost<<endl<<flush;
		if (TRACE)
		{
			fout<<"Server starting on host:"<<localhost<<endl;
		}
		//Create the socket
		if((mySocket = socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET) 
			throw "Socket failed\n";

		//Fill-in UDP Port and Address info.
		sa_in_local.sin_family = AF_INET;
		sa_in_local.sin_port = htons(sendPort);
		sa_in_local.sin_addr.s_addr = htonl(INADDR_ANY);

		//Bind the localhost's port
		if (bind(mySocket,(LPSOCKADDR)&sa_in_local,sizeof(sa_in_local)) == SOCKET_ERROR)
			throw "can't bind the socket";

		//Open the log file
		//fout.open(fn);
	}
	catch (char *str) 
	{
		cerr<<str<<":"<<dec<<WSAGetLastError()<<endl;
		exit(1);
	}

	srand( (unsigned)time( NULL ) );
	timeout.tv_sec=0;
	timeout.tv_usec=TIMEOUT_USEC;
	timeout_flag.tv_sec=2;
	timeout_flag.tv_usec=TIMEOUT_USEC;

	setBlock(FALSE);

}

//----------------------------------------------------
//Provide user interface
//----------------------------------------------------
void UDP::run()
{
	HOSTENT *rp;
	string remotehost;	
	do
	{
		cout<<endl<<"Input remote host to be connected(Server):";
		getline(cin,remotehost);
		if((rp=gethostbyname(remotehost.c_str())) == NULL)
		{
			cout<< "Connect to "<<remotehost<<" failed."<<endl;
		}
		else
		{
			break;
		}
	} while (true);
	
	memset(&sa_in_remote,0,sizeof(sa_in_remote));
	memcpy(&sa_in_remote.sin_addr,rp->h_addr,rp->h_length);
	sa_in_remote.sin_family = rp->h_addrtype;   
	sa_in_remote.sin_port = htons(requestPort);
	//Display the host machine internet address
	cout << "Connecting to remote host successfully:"<<endl
		<<"\t"<<inet_ntoa(sa_in_remote.sin_addr) << endl
		<<"\t"<<rp->h_name<<endl;
	if (TRACE)
	{
		fout<< "Connecting to remote host successfully:"<<endl
			<<"\t"<<inet_ntoa(sa_in_remote.sin_addr) << endl
			<<"\t"<<rp->h_name<<endl;
	}

	
	while(true)
	{
		if(threeWayHandShake())
		{
			if (event.command=="get")
			{
				if (TRACE)
				{
					fout<<"Getting file:"<<endl
						<<"file name:"<<event.fileName<<endl
						<<"file size:"<<event.size<<endl;
				}
				cout<<"Shaking hands done. File transmition begin......"<<endl;
				sendFile();
				cout<<"The file is sent."<<endl;
				if (TRACE)
				{
					fout<<"Getting file done."<<endl;
				}
			}
			else if (event.command=="put")
			{
				cout<<"Shaking hands done. File transmition begin......"<<endl;
				if (TRACE)
				{
					fout<<"Putting file:"<<endl
						<<"file name:"<<event.fileName<<endl
						<<"file size:"<<event.size<<endl;
				}
				recieveFile();
				if (TRACE)
				{
					fout<<"Getting file done."<<endl;
				}
				cout<<"The file is recieved."<<endl;
			}
			else
			{
				cout<<"Shaking hands done. Sending file list to the remote host......"<<endl
					<<"============================================="<<endl;
				string listStr=list();
				cout<<listStr<<endl;
				sendList(listStr);
				cout<<"The file list is sent."<<endl;
			}
		}
	}
	
}

//----------------------------------------------------
//three way hand shaking
//----------------------------------------------------
bool UDP::threeWayHandShake()
{
	Message sendMsg, getMsg;
	bool success=true;
	//Shake hands step 1
	//Recieve the first message which contains remote sequence number and command from remote host
	while(1)
	{
		fd_set readfds;				//fd_set is a type
		FD_ZERO(&readfds);			//initialize
		FD_SET(mySocket, &readfds); //put the socket in the set
		int outfds=0;
		outfds=select (1 , &readfds, NULL, NULL, &timeout);
		//check for incoming packets.
		if (outfds==SOCKET_ERROR)
		{
			throw "Timer error!";
		}			
		else if(outfds==0)			//Time out, waiting for new message.
		{}
		else						//There is an incoming packet.
		{
			try{
				getPacket(getMsg);
				remoteSeq=getMsg.seqNo1;				
				string request,data;
				//parse the recieved packet.
				for(int i = 0; i<getMsg.validDataLength; i++)
					request += getMsg.data[i];

				//get the command from the message
				int pos = request.find_first_of(';');
				event.command=request.substr(0,pos);

				request=request.substr(pos+1);	//remove the substring before the first ";", which is the command.
				//the command is "list"
				if (event.command=="list")
				{
					//Calculate files' size
					string fileList=list();
					event.size=fileList.length();
					data=intToString(event.size)+";";
				}
				else if(event.command=="put")	//put
				{
					pos = request.find_first_of(';');
					event.fileName=request.substr(0,pos);
					request=request.substr(pos+1);	//remove the substring before the first ";", which is the file name.
					pos = request.find_first_of(';');
					event.size=stringToInt(request.substr(0,pos));
				}
				else							//get
				{
					pos = request.find_first_of(';');
					event.fileName=request.substr(0,pos);	
					event.size=getFileSize(event.fileName);
					if (event.size==-1)
					{
						success=false;
					}
					data=intToString(event.size)+";";
				}

				sendMsg.validDataLength=data.length();
				memset(sendMsg.data,0,DATA_SIZE2);
				memcpy(sendMsg.data,data.c_str(),DATA_SIZE2);
				
				break;
			}
			catch (char *err)
			{
				cerr<<err<<endl;
			}
		}
	}
	//Shake hands step 2
	//Send feedback message,ACK, to the remote host.
	sendMsg.seqNo1=rand() % 256;
	localSeq=sendMsg.seqNo1;
	sendMsg.seqNo2=remoteSeq;
	
	guaranteedSendPacket(sendMsg,getMsg);

	//Get rid of the garbage packets
	do{
		int outfds=-1;
		fd_set readfds;				//fd_set is a type
		FD_ZERO(&readfds);			//initialize
		FD_SET(mySocket, &readfds); //put the socket in the set
		outfds = select (1 , &readfds, NULL, NULL, & timeout_flag);
		//check for incoming packets.
		if (outfds==SOCKET_ERROR)
		{
			throw "Timer error!";
		}
		//Time out, loop done.
		//No more garbage packets from the remote host
		else if(outfds==0)			
		{
			break;
		}
		else if (outfds == 1) {
			getPacket(getMsg);
		}
	}while(true);

	return success;
}

//----------------------------------------------------
//Send a file to the remote host
//----------------------------------------------------
void UDP::sendFile()
{
	ifstream file (event.fileName, std::ifstream::binary);
	if (file.is_open())
	{
		int recordCount=event.size/DATA_SIZE+1;
		int lastAckSeq=0;
		int seq;
		ACK_MSG ackMsg;
		MESSAGE message;

		int sendPacketCount=0;
		int recievePacketCount=0;
		while(lastAckSeq<recordCount)
		{
			seq=lastAckSeq+1;
			while(seq<=lastAckSeq+WINDOW_SIZE && seq<=recordCount)
			{
				int dataSize=DATA_SIZE;
				if (seq==recordCount)
				{
					dataSize=event.size-(seq-1) * DATA_SIZE;
				}
				int position=(seq-1) * DATA_SIZE;
				file.seekg (position, ios::beg);
				file.read (message.data, dataSize);
				message.validSize=dataSize;
				message.seq=seq;
				if(TRACE)
				{
					fout<<"Send packet with sequence number:"<<message.seq<<endl;
				}
				cout<<"Send packet with sequence number:"<<message.seq<<endl;
				sendPacket(message);
				sendPacketCount++;
				seq++;
			}

			int lastSentSeq=seq-1;

			while(true)
			{
				int outfds=0;
				fd_set readfds;				//fd_set is a type
				FD_ZERO(&readfds);			//initialize
				FD_SET(mySocket, &readfds); //put the socket in the set
				outfds=select (1 , &readfds, NULL, NULL, &timeout);
				if (outfds==SOCKET_ERROR)
				{
					throw "Timer error!";
				}
				else if(outfds==0)			//Time out, resent packets
				{
					break;
				}
				else						//There is an incoming packet.
				{
					getPacket(ackMsg);
					recievePacketCount++;
					if(TRACE)
					{
						fout<<"Recieved "<<(ackMsg.flag?"an ACK":"a NACK")<<" with sequence number of "<<ackMsg.seq<<endl;
					}
					cout<<"Recieved "<<(ackMsg.flag?"an ACK":"a NACK")<<" with sequence number of "<<ackMsg.seq<<endl;
					if(ackMsg.flag)			//ACK
					{
						if (ackMsg.seq>lastAckSeq)
						{
							lastAckSeq=ackMsg.seq;
							if (lastSentSeq==lastAckSeq)
							{break;}
						}
					}
					else					//NACK
					{
						lastAckSeq=ackMsg.seq-1;
						break;
					}
				}
			}
		}
		file.close();
		//Deal with garbage packets
		while (true)
		{
			int outfds=0;
			fd_set readfds;				//fd_set is a type
			FD_ZERO(&readfds);			//initialize
			FD_SET(mySocket, &readfds); //put the socket in the set
			outfds=select (1 , &readfds, NULL, NULL, &timeout_flag);
			if (outfds==SOCKET_ERROR)
			{
				throw "Timer error!";
			}			
			else if(outfds==0)			//Time out, no more garbage message comes.
			{
				break;
			}
			else						//There is a garbage message.
			{
				getPacket(ackMsg);
				recievePacketCount++;
				if(TRACE)
				{
					fout<<"Recieved "<<(ackMsg.flag?"an ACK":"a NACK")<<" with sequence number of "<<ackMsg.seq<<endl;
				}
				cout<<"Recieved "<<(ackMsg.flag?"an ACK":"a NACK")<<" with sequence number of "<<ackMsg.seq<<endl;
			}
		}
		if (TRACE)
		{
			fout<<"The amount of sent packets:"<<sendPacketCount<<endl;
			fout<<"The amount of recieved packets:"<<recievePacketCount<<endl;
			fout<<"The amount of valid packets:"<<recordCount<<endl;
		}
		cout<<"The amount of sent packets:"<<sendPacketCount<<endl;
		cout<<"The amount of recieved packets:"<<recievePacketCount<<endl;
		cout<<"The amount of valid packets:"<<recordCount<<endl;
	}
	else
		throw "Open file error.";	
}

//----------------------------------------------------
//Recieve a file from the remote host
//----------------------------------------------------
void UDP::recieveFile()
{
	ofstream file(event.fileName, ios::binary);
	if(file.is_open())
	{
		int recordCount=event.size/DATA_SIZE+1;
		int expectedSeq=1;
		ACK_MSG ackMsg;
		MESSAGE message;
		int sendPacketCount=0;
		int recievePacketCount=0;
		while (true)
		{
			if(TRACE)
			{
				fout<<"Expected sequence number: "<<expectedSeq<<endl;
			}
			cout<<"Expected sequence number: "<<expectedSeq<<endl;
			int outfds=0;
			fd_set readfds;				//fd_set is a type
			FD_ZERO(&readfds);			//initialize
			FD_SET(mySocket, &readfds); //put the socket in the set
			outfds=select (1 , &readfds, NULL, NULL, &timeout);
			if (outfds==SOCKET_ERROR)
			{
				throw "Timer error!";
			}			
			else if(outfds==0)			//Time out, send a NACK
			{
				if(expectedSeq>recordCount)
					break;
				else
				{
					ackMsg.flag=NACK;
					ackMsg.seq=expectedSeq;
				}
			}
			else						//There is an incoming packet.
			{
				int remoteSize=sizeof(sa_in_remote);
				getPacket(message);
				recievePacketCount++;
				if(TRACE)
				{
					fout<<"Received packet with sequence number: "<<message.seq<<endl;
				}
				cout<<"Received packet with sequence number: "<<message.seq<<endl;
				if(message.seq==expectedSeq)
				{
					ackMsg.flag=ACK;
					ackMsg.seq=expectedSeq;
					file.write(message.data,message.validSize);
					expectedSeq++;
				}
				else
				{
					ackMsg.flag=NACK;
					ackMsg.seq=expectedSeq;
				}
			}
			if(TRACE)
			{
				fout<<"Send packet with sequence number:"<<ackMsg.seq<<endl;
			}
			cout<<"Send packet with sequence number:"<<ackMsg.seq<<endl;
			sendPacket(ackMsg);
			sendPacketCount++;
		}
		file.close();

		//get rid of garbage packet
		while (true)
		{
			int outfds=0;
			fd_set readfds;				//fd_set is a type
			FD_ZERO(&readfds);			//initialize
			FD_SET(mySocket, &readfds); //put the socket in the set
			outfds=select (1 , &readfds, NULL, NULL, &timeout_flag);
			if (outfds==SOCKET_ERROR)
			{
				throw "Timer error!";
			}			
			else if(outfds==0)			//Time out, no more garbage message comes
			{
				break;
			}
			else						//There is a garbage message.
			{
				getPacket(message);
				recievePacketCount++;
				if(TRACE)
				{
					fout<<"Received packet with sequence number: "<<message.seq<<endl;
				}
				cout<<"Received packet with sequence number: "<<message.seq<<endl;
			}
		}
		if (TRACE)
		{
			fout<<"The amount of sent packets:"<<sendPacketCount<<endl;
			fout<<"The amount of recieved packets:"<<recievePacketCount<<endl;
			fout<<"The amount of valid packets:"<<recordCount<<endl;
		}
		cout<<"The amount of sent packets:"<<sendPacketCount<<endl;
		cout<<"The amount of recieved packets:"<<recievePacketCount<<endl;
		cout<<"The amount of valid packets:"<<recordCount<<endl;
	}
	else
		throw "Cannot open this file";
}

//----------------------------------------------------
//A packet is guaranteed to be sent to a remote host
//----------------------------------------------------
void UDP::guaranteedSendPacket(Message &sendMsg, Message &getMsg)
{
	string request,localNum,message;
	
	sendPacket(sendMsg);
	int outfds=0;
	do{
		fd_set readfds;				//fd_set is a type
		FD_ZERO(&readfds);			//initialize
		FD_SET(mySocket, &readfds); //put the socket in the set
		outfds=select (1 , &readfds, NULL, NULL, &timeout);
		//check for incoming packets.
		if (outfds==SOCKET_ERROR)
		{
			throw "Timer error!";
		}			
		else if(outfds==0)			//Time out, resent packet
		{
			sendPacket(sendMsg);				
		}
		else						//There is an incoming packet.
		{
			try
			{
				getPacket(getMsg);
				//If an expected packet recieved then end the loop
				if (localSeq==getMsg.seqNo2)
				{
					remoteSeq=getMsg.seqNo1;
					break;
				}
				//Recieve a unexpected packet, resent.
				else
				{
					sendPacket(sendMsg);
				}
			}
			catch (char *err)
			{
				cerr<<err<<endl;
			}
		}
	}while(true);
}

//----------------------------------------------------
//Get the file list of the localhost
//----------------------------------------------------
string UDP::list()
{
	string fileList;
	HANDLE hFind;
	WIN32_FIND_DATA data;
 
	wchar_t dirN[25]=_T(".\\*.*");
	hFind = FindFirstFile(dirN, &data);
	if (hFind != INVALID_HANDLE_VALUE) 
 	{
		do 
		{
			char fileN[256];
			size_t   i;					
 
			wcstombs_s(&i, fileN, sizeof(data.cFileName), data.cFileName, sizeof(data.cFileName));						

			for(char *p=fileN; *p; p++)
				fileList+=*p;
			fileList+='\n';						
		} while (FindNextFile(hFind, &data));//find the next file after call the FindFirstFile
 
		FindClose(hFind);
	}
	return fileList;
}


//----------------------------------------------------
//Sent a the file list to a remote host
//----------------------------------------------------
void UDP::sendList(string listStr)
{
	Message sendMsg,getMsg;
	sendMsg.seqNo1=rand() % 256;
	localSeq=sendMsg.seqNo1;
	sendMsg.seqNo2=remoteSeq;
	sendMsg.validDataLength=listStr.length();
	memset(sendMsg.data,0,DATA_SIZE);
	memcpy(sendMsg.data,listStr.c_str(),DATA_SIZE);
	guaranteedSendPacket(sendMsg,getMsg);
	//Get rid of the garbage packets
	do{
		int outfds=-1;
		fd_set readfds;				//fd_set is a type
		FD_ZERO(&readfds);			//initialize
		FD_SET(mySocket, &readfds); //put the socket in the set
		outfds = select (1 , &readfds, NULL, NULL, & timeout_flag);
		//check for incoming packets.
		if (outfds==SOCKET_ERROR)
		{
			throw "Timer error!";
		}
		//Time out, loop done.
		//No more garbage packets from the remote host
		else if(outfds==0)			
		{
			break;
		}
		else if (outfds == 1) {
			getPacket(getMsg);
		}
	}while(true);
}

//----------------------------------------------------
//Sent a message with type of MESSAGE to a remote host
//----------------------------------------------------
void UDP::sendPacket(MESSAGE &msg)
{
	if(sendto(mySocket, (char *)&msg, MSG_SIZE,0,(SOCKADDR*)&sa_in_remote,sizeof(sa_in_remote))==SOCKET_ERROR)
		throw "Send packet error!";
}

//----------------------------------------------------
//Sent a message with type of Message to a remote host
//----------------------------------------------------
void UDP::sendPacket(Message &msg)
{
	if(sendto(mySocket, (char *)&msg, MSG_SIZE,0,(SOCKADDR*)&sa_in_remote,sizeof(sa_in_remote))==SOCKET_ERROR)
		throw "Send packet error!";
}

//----------------------------------------------------
//Sent a message with type of ACK_MSG to a remote host
//----------------------------------------------------
void UDP::sendPacket(ACK_MSG &ackMsg)
{
	if(sendto(mySocket, (char *)&ackMsg, MSG_SIZE,0,(SOCKADDR*)&sa_in_remote,sizeof(sa_in_remote))==SOCKET_ERROR)
		throw "Send packet error!";
}

//----------------------------------------------------
//Recieve a message with type of MESSAGE from a remote host
//----------------------------------------------------
void UDP::getPacket(MESSAGE &msg)
{
	int remoteSize=sizeof(sa_in_remote);
	if(recvfrom(mySocket, (char*)&msg, MSG_SIZE, 0, (SOCKADDR *)&sa_in_remote, &remoteSize)==SOCKET_ERROR)
		throw "Get packet error!";
}

//----------------------------------------------------
//Recieve a message with type of ACK_MSG from a remote host
//----------------------------------------------------
void UDP::getPacket(ACK_MSG &ackMsg)
{
	int remoteSize=sizeof(sa_in_remote);
	if(recvfrom(mySocket, (char*)&ackMsg, MSG_SIZE, 0, (SOCKADDR *)&sa_in_remote, &remoteSize)==SOCKET_ERROR)
		throw "Get packet error!";
}

//----------------------------------------------------
//Recieve a message with type of Message from a remote host
//----------------------------------------------------
void UDP::getPacket(Message &msg)
{
	int remoteSize=sizeof(sa_in_remote);
	if(recvfrom(mySocket, (char*)&msg, MSG_SIZE, 0, (SOCKADDR *)&sa_in_remote, &remoteSize)==SOCKET_ERROR)
		throw "Get packet error!";
}

//----------------------------------------------------
//Get a file's size
//----------------------------------------------------
int UDP::getFileSize(string filename){
	int size = 0;
	ifstream infile(filename.c_str());
	if(infile.is_open())
	{
		infile.seekg(0, ios::end ); // move to end of file
		size =(int) infile.tellg();
	}else
	{
		cout<< "Could not open file\n";
		return -1;
	}
	return size;
}

//----------------------------------------------------
//Conver an interger to a string
//----------------------------------------------------
string UDP::intToString(int n)
{	
	stringstream out;
	out<<n;
	return out.str();
}

//----------------------------------------------------
//Conver a string to an integer
//----------------------------------------------------
int UDP::stringToInt(string s)
{	
	int n;
	stringstream in;
	in<<s;
	in>>n;
	return n;
}

//----------------------------------------------------
//Set the block way for a timer
//----------------------------------------------------
DWORD UDP::setBlock(bool behavior)
{
	DWORD dwBytesReturned = 0;
	DWORD status;
 
	status = WSAIoctl(mySocket, SIO_UDP_CONNRESET,
                &behavior,
                sizeof (behavior),
                NULL, 0, &dwBytesReturned,
                NULL, NULL);
	return status;
}

//----------------------------------------------------
//Distructor
//----------------------------------------------------
UDP::~UDP()
{
	closesocket(mySocket);
	WSACleanup();  
	fout.close();
}
