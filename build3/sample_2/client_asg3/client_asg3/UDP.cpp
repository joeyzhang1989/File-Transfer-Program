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
		{
			throw "Error in starting WSAStartup()\n";
		}

		HOSTENT *hp;
		gethostname(localhost,MAXHOSTNAMELEN);
		if((hp=gethostbyname(localhost)) == NULL) 
			throw "gethostbyname failed\n";
		cout<<"Client starting on host:"<<localhost<<endl<<flush;
		if (TRACE)
		{
			fout<<"Client starting on host:"<<localhost<<endl;
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
	}
	catch (char *str) 
	{
		cerr<<str<<":"<<dec<<WSAGetLastError()<<endl;
		exit(1);
	}

	srand( (unsigned)time( NULL ) );
	timeout.tv_sec=5;
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
	string command;
	do
	{		
		cout<<endl
			<<"1. Send a file to the remote host."<<endl
			<<"2. Get a file from the remote host."<<endl
			<<"3. List the files on the remote host."<<endl
			<<"4. List the files on the local host."<<endl
			<<"5. Quit "<<endl
			<<"---------------------------------"<<endl
			<<"Input your option[1-5]: ";
		
		std::getline(std::cin,command);
		if(command=="1")	//Put
		{
			event.command="put";
			event.fileName=inputFileName();
			ifstream file(event.fileName.c_str());
			if (file.is_open())
			{
				file.seekg(0, ios::end ); // move to end of file
				event.size =(int) file.tellg();
				file.close();				
				try{
					if(threeWayHandShake())
					{
						cout<<"Shaking hands done. File transmition begin......"<<endl;
						if (TRACE)
						{
							fout<<"Putting file:"<<endl
								<<"file name:"<<event.fileName<<endl
								<<"file size:"<<event.size<<endl;
						}
						cout<<"Putting file:"<<endl
								<<"file name:"<<event.fileName<<endl
								<<"file size:"<<event.size<<endl;
						sendFile();
						cout<<"The file is sent."<<endl;
						if (TRACE)
						{
							fout<<"Putting file done."<<endl;
						}
					}
				}catch(char *err){
					cerr<<err<<endl;
				}
			}
			else
			{
				cout<<"Open "<<event.fileName<<" failed"<<endl;
			}
		}
		else if(command=="2")		//Get
		{
			event.command="get";
			event.fileName=inputFileName();
			try{
				if(threeWayHandShake())
				{
					cout<<"Shaking hands done. File transmition begin......"<<endl;
					if (TRACE)
					{
						fout<<"Geting file:"<<endl
							<<"file name:"<<event.fileName<<endl
							<<"file size:"<<event.size<<endl;
					}
					recieveFile();
					cout<<"The file is recieved."<<endl;
					if (TRACE)
					{
						fout<<"Getting file done."<<endl;
					}
				}
			}catch(char *err){
				cerr<<err<<endl;
			}
		}
		else if(command=="3")		//List remote files.
		{
			event.command="list";
			try{
				if(threeWayHandShake())
				{
					cout<<"Files on remote host."<<endl
						<<"============================================="<<endl;
					listRemoteFiles();
				}
			}catch(char *err){
				cerr<<err<<endl;
			}
		}
		else if(command=="4")		//List local files.
		{
			cout<<"Files on local host."<<endl
				<<"============================================="<<endl
				<<list()<<endl;
		}
		else if(command=="5")		//Quit
		{
			break;
		}
		else
		{
			cout<<endl<<"A wrong option was chosen. Try again!"<<endl
				<<"============================="<<endl<<endl;
		}
	} while (true);
	
}
//----------------------------------------------------
//three way hand shaking
//----------------------------------------------------
bool UDP::threeWayHandShake()
{
	Message sendMsg,getMsg;
	bool success=true;
	//Shake hands step 1
	//Send the first message, which includes the local sequence number, and the event to the remote host
	localSeq=rand() % 256;
	sendMsg.seqNo1=localSeq;
	sendMsg.seqNo2=-1;
	string data=event.command;

	if (event.command=="put")
	{
		data=data+";"+event.fileName+";"+intToString(event.size);
	}
	else if(event.command=="get")
	{
		data=data+";"+event.fileName;
	}
	sendMsg.validDataLength=data.length();
	memset(sendMsg.data,0,DATA_SIZE2);
	memcpy(sendMsg.data,data.c_str(),DATA_SIZE2);

	guaranteedSendPacket(sendMsg,getMsg);

	//Shake hands step 2
	//Parse the recieved message

	string request;
	remoteSeq=getMsg.seqNo1;

	for(int i = 0; i<getMsg.validDataLength; i++)
		request += getMsg.data[i];

	//If the request contains a ";", which means the command is either "get" or "list"
	//If it is "get", then parse the file size.
	//If it is "list", then parse the files information size.
	int pos = request.find_first_of(';');
	
	if(event.command=="list"){
		event.size=stringToInt(request.substr(0,pos));
	}
	if(event.command=="get"){
		event.size=stringToInt(request.substr(0,pos));
		if(event.size==-1)
		{
			cout<<"No such file or open file failed on the remote host."<<endl;
			success=false;
		}
	}

	//Shake hands step 3
	//Send the second message to the remote host, which is a confirm message.
	localSeq=rand() % 256;
	sendMsg.seqNo1=localSeq;
	sendMsg.seqNo2=remoteSeq;
	sendMsg.validDataLength=0;
	memset(sendMsg.data,0,DATA_SIZE);

	sendPacket(sendMsg);
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
		//No more request from the remote host means the second packet is transmitted properly to the remote host,
		else if(outfds==0)			
		{
			break;
		}
		else if (outfds == 1) {
			getPacket(getMsg);
			sendPacket(sendMsg);
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
//List the files of a remote host
//----------------------------------------------------
void UDP::listRemoteFiles()
{
	Message sendMsg,getMsg;
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
		{
		}
		else						//There is an incoming packet.
		{
			try{
				//cout<<"recieve a message in the list"<<endl;
				getPacket(getMsg);
				if(localSeq==getMsg.seqNo2)
				{
					//cout<<"expected packet."<<endl;
					remoteSeq=getMsg.seqNo1;				
					string request,data;
					//parse the recieved packet.
					for(int i = 0; i<getMsg.validDataLength; i++)
						request += getMsg.data[i];
					cout<<request<<endl;
					localSeq=rand() % 256;
					sendMsg.seqNo1=localSeq;
					sendMsg.seqNo2=remoteSeq;
					sendPacket(sendMsg);
					break;
				}
			}
			catch (char *err)
			{
				cerr<<err<<endl;
			}
		}
	}
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
		//No more request from the remote host means the second packet is transmitted properly to the remote host,
		else if(outfds==0)			
		{
			break;
		}
		else if (outfds == 1) {
			getPacket(getMsg);
			sendPacket(sendMsg);
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
			//cout<<"There are incoming packets"<<endl;
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
		cout<< "could not open file\n";
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
//Accept a keyboard input which is supposed to be file name
//----------------------------------------------------
string UDP::inputFileName()
{
	string fileName;
	cout<<endl<<"Input a file to be transmitted:";
	getline(cin,fileName);
	return fileName;
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
//Distructor
//----------------------------------------------------
UDP::~UDP()
{
	closesocket(mySocket);
	WSACleanup();  
	fout.close();
}

