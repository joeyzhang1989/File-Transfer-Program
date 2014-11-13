#include "client.h"
string Client::getFileSize(string filename){
	string size="";
	string fullFilePath= filePath+filename;
	int fileSize = 0;
	stringstream out;
	ifstream infile(fullFilePath.c_str());
	if(infile.is_open())
	{
		infile.seekg(0, ios::end ); // move to end of file
		fileSize = infile.tellg();
		out<<fileSize;
		size=out.str();
	}else
	{
		cout<< "could not open file\n";
	}
	return size;
}

void Client::receiveFile(string filename){
	int totalNumOfBytes=0;
	int ibytesrecv=0;
	if(TRACE)
	{
		fout<<"Receiver: starting on host "<<localhost<<endl;
	}
	string msg="";
	string fullFilePath;
	segment_msg msgR;
	int num_of_file_packet=(atoi(serv_size.c_str())/SIZE)+1;
	FILE *stream; string ack;
	fullFilePath=filePath+filename;
	if( (stream = fopen( fullFilePath.c_str(), "wb" )) != NULL ) 
	{
		if(TRACE)
		{
			fout<<"Receiver: receiving file "<<fullFilePath<<endl;
		}
		for(int i=0;i<num_of_file_packet;i++)
		{
			//receive from sockets
			do{

				ibytesrecv = recvfrom(mySocket, (char*)&msgR, sizeof(msgR), 0, (struct sockaddr *)&sa_in, &server_length);
				int seq=msgR.seqNo;
				if(TRACE)
				{
					fout<<"Receiver: received packet "<<seq<<endl;
				}
				if(ibytesrecv>0)
				{
					stringstream out;
					current_Recv_Seq=msgR.seqNo;
					out<<current_Recv_Seq;
					ack="ACK"+out.str();
					memset(szbuffer,0,SIZE);
					memcpy(szbuffer,ack.c_str(),SIZE);
					// if seq number=next expected seq number,write to file and send ack
					if(current_Recv_Seq==next_Rec_Seq)
					{
						fwrite((char*)msgR.data, sizeof(char), ibytesrecv-sizeof(int), stream );
						if (sendto(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, server_length) == -1)
							fprintf(stderr, "Error transmitting data.\n");
						totalNumOfBytes=totalNumOfBytes+ibytesrecv-sizeof(int);
						if(TRACE)
						{
							fout<<"Receiver: sent ACK for packet "<<seq<<endl;
						}
					}
					// else if seq number =previous recvd seq number,send ack
					else 
					{
						if(TRACE)
						{
							fout<<"Receiver: received same packet "<<seq<<endl;
						}
						if (sendto(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, server_length) == -1)
							fprintf(stderr, "Error transmitting data.\n");
						if(TRACE)
						{
							fout<<"Receiver: sent ACK for packet "<<seq<<endl;
						}
					}
				}
				else 
				{
					break;
				}
			}while(current_Recv_Seq==prev_Recv_Seq);
			prev_Recv_Seq=current_Recv_Seq;
			next_Rec_Seq= abs(next_Rec_Seq-1);
		}
		cout<<"Transfer complete \n";
		if(TRACE)
		{
			fout<<"Sender: File transfer completed"<<endl;
			fout<<"Sender: Number of bytes received "<<totalNumOfBytes<<endl;
		}
	}
	else
	{
		cout<<"Problem creating the file to be written\n" ;
	}
	fclose( stream );
	system("pause");
}

void Client::sendFile(string filename)
{
	int totalNumOfPackets=0;
	int ibytesrecv=0;
	FILE *file;      // File pointer.
	string fullFilePath;
	fullFilePath = filePath+filename;
	cout<<"sending file"<<filename<<endl;
	int MAX_SIZE = SIZE;//maximum size of data to be read at a time
	file=fopen(fullFilePath.c_str(),"rb");
	if(TRACE)
	{
		fout<<"Sender: starting on host "<<localhost<<endl;
	}
	if(file!=NULL)
	{
		if(TRACE)
		{
			fout<<"Sender: sending file "<<fullFilePath<<endl;
		}
		segment_msg msg;
		int num_of_file_packet=0,num_of_read=0;
		while(!feof(file))
		{
			num_of_file_packet=num_of_file_packet+1;//the file's packet.
			memset(msg.data,0,SIZE);
			num_of_read=fread(msg.data,sizeof(char),MAX_SIZE,file);
			msg.seqNo=Send_Seq;

			int lastPeriodPosition=filename.find_last_of('.');
			string extFileName=filename.substr(lastPeriodPosition+1);
			if (extFileName=="txt")
			{
				cout<<msg.data<<endl;
			}

			//send to receiver with currentSendSeqNo
			if (sendto(mySocket,(char*)&msg, sizeof(msg), 0, (struct sockaddr *)&sa_in, server_length) == -1)
				fprintf(stderr, "Error transmitting data.\n");
			if(TRACE)
			{
				fout<<"Sender: sent packet "<<Send_Seq<<endl;
			}
			totalNumOfPackets++;
			//wait for ack.
			do{
				fd_set readfds; //fd_set is a type
				FD_ZERO(&readfds); //initialize
				FD_SET(mySocket, &readfds); //put the socket in the set
				if(!(outfds = select (1 , &readfds, NULL, NULL, & timeout)))
				{//if timed out, resent
					if (sendto(mySocket,(char*)&msg, sizeof(msg), 0, (struct sockaddr *)&sa_in, server_length) == -1)
						fprintf(stderr, "Error transmitting data.\n");
					if(TRACE)
					{
						fout<<"Sender: resent packet "<<Send_Seq<<endl;
					}
					totalNumOfPackets++;
				}
				if (outfds == 1) 
				{//if not timed out, receive ack, store ack seq number
					memset(szbuffer,0,SIZE);
					ibytesrecv = recvfrom(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, &server_length);
					if(TRACE)
					{
						fout<<"Sender: received ACK for packet "<<Send_Seq<<endl;
					}
				}
			}while(outfds!=1);
			Send_Seq= abs(Send_Seq-1);
		}
		cout<<"File transfer completed\n";
		if(TRACE)
		{
			fout<<"Sender: File transfer completed"<<endl;
			fout<<"Sender: Number of packets sent "<<totalNumOfPackets<<endl;
			fout<<"Sender: Number of effective packets sent "<<num_of_file_packet<<endl;
		}
	}
	else
	{
		cout<<"could not open "<<file<<" for reading\n";
	}
	fclose(file);//close file after reading      
}

bool Client::threeWayHandShake()
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
		fprintf(stderr, "Error transmitting data.\n");
	do{
		fd_set readfds; //fd_set is a type
		FD_ZERO(&readfds); //initialize
		FD_SET(mySocket, &readfds); //put the socket in the set
		if(!(outfds = select (1 , &readfds, NULL, NULL, & timeout))) 
		{//timed out, return
			if (sendto(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, server_length) == -1)
				fprintf(stderr, "Error transmitting data.\n");
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
		fprintf(stderr, "Error transmitting data.\n");
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
				fprintf(stderr, "Error transmitting data.\n");
		}
	}while(success!=true);
	server_Start_Seq=atoi(serverNum.c_str())%2;
	client_Start_Seq=atoi(randomNum.c_str())%2;
	Send_Seq=client_Start_Seq;
	next_Rec_Seq=server_Start_Seq;
	return success;
}

void Client::listClientFiles()
 {
	char endChar[5]="\n";
	HANDLE hFind;
	WIN32_FIND_DATA data;
 
	wchar_t dirN[25]=_T(".\\*.*");
	char *FileList = new char[SIZE];

	memset(FileList, '\0', SIZE);
	hFind = FindFirstFile(dirN, &data);
	if (hFind != INVALID_HANDLE_VALUE) 
 	{
		do 
		{
				char fileN[256];
				size_t   i;					
 
				wcstombs_s(&i, fileN, sizeof(data.cFileName), data.cFileName, sizeof(data.cFileName));						
				// cout<<"\n"<< fileN;
 
				if(strlen(FileList)<=0)
				{
					strcpy_s(FileList, SIZE, fileN);//if there is nothing in FileList, add the first one into it.
				}
				else
				{
				// cout<<"Here3" <<endl;
 				strcat_s(FileList, SIZE,endChar);//endChar="\n". if there is a filename in the filelist, add a space line
				strcat_s(FileList, SIZE,fileN);//then add the name behind the exits names.
 				}						
		} while (FindNextFile(hFind, &data));//find the next file after call the FindFirstFile
 
		FindClose(hFind);
	}	

	//Display fileList
	cout<<"=============The files on client====================="<<endl
		<<FileList<<endl
		<<"====================================================="<<endl<<endl;
	delete FileList;
}

void Client::listServerFiles()
{
	int ibytesrecv=0;
	segment_msg msgl;
	string ack;
	if(TRACE)
	{
		fout<<"Receiver: starting on host "<<localhost<<endl;
		fout<<"Receiver: receiving server file list "<<endl;
	}
	do{
		memset(msgl.data,0,512);
		ibytesrecv = recvfrom(mySocket, (char*)&msgl, sizeof(msgl), 0, (struct sockaddr *)&sa_in, &server_length);
		int seq=msgl.seqNo;
		if(TRACE)
		{
			fout<<"Receiver: received packet "<<seq<<endl;
		}	
		if(ibytesrecv>0)
		{
			stringstream out;
			current_Recv_Seq=msgl.seqNo;
			out<<current_Recv_Seq;
			ack="ACK"+out.str();
			memset(szbuffer,0,SIZE);
			memcpy(szbuffer,ack.c_str(),SIZE);

			if(current_Recv_Seq==next_Rec_Seq)
			{
		
				if (sendto(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, server_length) == -1)
					fprintf(stderr, "Error transmitting data.\n");
		
				if(TRACE)
				{
					fout<<"Receiver: sent ACK for packet "<<seq<<endl;
				}
			}	
			else 
			{
				if(TRACE)
				{
					fout<<"Receiver: received same packet "<<seq<<endl;
				}
				if (sendto(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, server_length) == -1)
					fprintf(stderr, "Error transmitting data.\n");
				if(TRACE)
				{
					fout<<"Receiver: sent ACK for packet "<<seq<<endl;
				}
			}
		}
		else 
		{
			break;
		}
	}while(current_Recv_Seq==prev_Recv_Seq);
	prev_Recv_Seq=current_Recv_Seq;
	next_Rec_Seq= abs(next_Rec_Seq-1);
	cout<<"=============The files on server====================="<<endl
		<<(char*)&msgl<<endl
		<<"====================================================="<<endl<<endl;	

	if(TRACE)
	{
		fout<<"Receiver: File list transfer completed"<<endl;		
	}
}

void Client::deleteClientFiles()
{
	char filepath[FILELENGTH];
	char filename[FILELENGTH];
	_getcwd(filepath, FILELENGTH);//return the currentpath to the buffer if fail will return false
	strcat(filepath, "\\");
	cout << "enter the name of file that you want to delete:" << endl;
	cin >> filename;
	if (!_access(filename, 00))//if file exist 
	{
		if (remove(filename) == -1)
		{
			cout << "Could not delete :" << filename << endl;
		}
		else
		{
			cout << "delete :" << filename << "   successful" << endl;
		}

	}
	else//file dose not exist
	{
		cout << filename << ":  file dose not exist" << endl;
	}

}

void Client::sendControl(string file, string dir)
{
	string con_msg,size;
	segment_msg msg;
	int ibytesrecv=0;
	if(dir=="put")
	{
		size=getFileSize(file);
		con_msg=dir+","+file+","+size;
	}
	else if(dir=="list")
	{
		con_msg=dir;
	}
	else if(dir == "get")
	{
		con_msg=dir+","+file;
	}
	else if (dir == "delete")
	{
		con_msg=dir+","+file+",";
	}
	memset(msg.data,0,SIZE);
	memcpy(msg.data,con_msg.c_str(),SIZE);
	msg.seqNo=Send_Seq;
	
	if (sendto(mySocket,(char*)&msg, sizeof(msg), 0, (struct sockaddr *)&sa_in, server_length) == -1)
		fprintf(stderr, "Error transmitting data.\n");
	
	do{
		fd_set readfds; //fd_set is a type
		FD_ZERO(&readfds); //initialize
		FD_SET(mySocket, &readfds); //put the socket in the set
		if(!(outfds = select (1 , &readfds, NULL, NULL, & timeout)))
		{//timed out                   
			if (sendto(mySocket,(char*)&msg, sizeof(msg), 0, (struct sockaddr *)&sa_in, server_length) == -1)
				fprintf(stderr, "Error transmitting data.\n");								
		}
		if (outfds == 1) 
		{
			memset(szbuffer,0,512);
			ibytesrecv = recvfrom(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, &server_length);
		}
	}while(outfds!=1);

	string ser_msg;
	for(int i = 0; szbuffer[i] != 0; i++)
		ser_msg += szbuffer[i];
	int pos = ser_msg.find_first_of(",");

	if(pos!=string::npos)
	{
		serv_size = ser_msg.substr(0,pos);
	}

	Send_Seq= abs(Send_Seq-1);
}

void Client::deleteServerFiles()
{
	int ibytesrecv=0;
	segment_msg msgl;
	string ack;
	
	if(TRACE)
	{
		fout<<"Receiver: starting on host "<<localhost<<endl;
		fout<<"Receiver: receiving delete message "<<endl;
	}
	do{
		memset(msgl.data,0,512);
		ibytesrecv = recvfrom(mySocket, (char*)&msgl, sizeof(msgl), 0, (struct sockaddr *)&sa_in, &server_length);
		int seq=msgl.seqNo;
		if(TRACE)
		{
			fout<<"Receiver: received packet "<<seq<<endl;
			fout<<msgl.data<<endl;
		}
		if(ibytesrecv>0)
		{
			stringstream out;
			current_Recv_Seq=msgl.seqNo;
			out<<current_Recv_Seq;
			ack="ACK"+out.str();
			memset(szbuffer,0,SIZE);
			memcpy(szbuffer,ack.c_str(),SIZE);

			if(current_Recv_Seq==next_Rec_Seq)
			{
		
				if (sendto(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, server_length) == -1)
					fprintf(stderr, "Error transmitting data.\n");
		
				if(TRACE)
				{
					fout<<"Receiver: sent ACK for packet "<<seq<<endl;
				}
			}	
			else 
			{
				if(TRACE)
				{
					fout<<"Receiver: received same packet "<<seq<<endl;
				}
				if (sendto(mySocket, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa_in, server_length) == -1)
					fprintf(stderr, "Error transmitting data.\n");
				if(TRACE)
				{
					fout<<"Receiver: sent ACK for packet "<<seq<<endl;
				}
			}
		}
	}while(current_Recv_Seq==prev_Recv_Seq);
	prev_Recv_Seq=current_Recv_Seq;
	next_Rec_Seq= abs(next_Rec_Seq-1);
	cout<<"=============message from the server====================="<<endl
		<<(char*)&msgl<<endl
		<<"====================================================="<<endl<<endl;	

	if(TRACE)
	{
		fout<<"Receiver: File delete completed"<<endl;		
	}


}
void Client::run()
{
	WSADATA wsadata;
	HOSTENT *rp;
	fout.open(fn);
	try 
	{
		if (WSAStartup(0x0202,&wsadata)!=0)
		{  
			cout<<"Error in starting WSAStartup()" << endl;
		} 
		else 
		{
			char* buffer="WSAStartup was successful\n";   

			WriteFile(test,buffer,sizeof(buffer),&dwtest,NULL); 
			/* Display the wsadata structure */
			cout<< endl
				<<"==============The wsadata structure==================="<<endl
				<< "wsadata.wVersion "       << wsadata.wVersion       << endl
				<< "wsadata.wHighVersion "   << wsadata.wHighVersion   << endl
				<< "wsadata.szDescription "  << wsadata.szDescription  << endl
				<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
				<< "wsadata.iMaxSockets "    << wsadata.iMaxSockets    << endl
				<< "wsadata.iMaxUdpDg "      << wsadata.iMaxUdpDg      << endl
				<<"======================================================"<<endl;
		}  
		//Display name of local host.
		gethostname(localhost,21);
		cout<<"Local host name is \"" << localhost << "\"" << endl;
		HOSTENT *hp;
		if((hp=gethostbyname(localhost)) == NULL) 
			throw "gethostbyname failed\n";
		//Create the socket
		if((mySocket = socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET) 
			throw "Socket failed\n";


		DWORD dwBytesReturned = 0;
		BOOL  bNewBehavior = FALSE;
		DWORD status;
 
		status = WSAIoctl(mySocket, SIO_UDP_CONNRESET,
                   &bNewBehavior,
                   sizeof (bNewBehavior),
                   NULL, 0, &dwBytesReturned,
                   NULL, NULL);
		if (SOCKET_ERROR == status)
		{
			DWORD dwErr = WSAGetLastError();
			if (WSAEWOULDBLOCK == dwErr)
			{
				//return(FALSE);
			}
			else
			{
				printf("WSAIoctl(SIO_UDP_CONNRESET) Error: %d/n", dwErr);
				//return(FALSE);
			}
		} 

		//Fill-in Client port and Address info.
		sa.sin_family = AF_INET;
		sa.sin_port = htons(SEND_PORT);
		sa.sin_addr.s_addr = htonl(INADDR_ANY);
		//Bind the server port
		if (bind(mySocket,(LPSOCKADDR)&sa,sizeof(sa)) == SOCKET_ERROR)
			throw "can't bind the socket";
		cout << "Bind was successful" << endl;
		//Ask for name of remote server
		server_length = sizeof(struct sockaddr_in);
		timeout.tv_sec=SECT;
		timeout.tv_usec=USEC;
		while(true)
		{
			cout << "please enter your router name :" << flush ;   
			cin >> remotehost ;

			cout << "Router name is: \"" << remotehost << "\"" << endl;
			if((rp=gethostbyname(remotehost)) == NULL)
			{
				cout<< "remote gethostbyname failed\n";
				continue;
			}
			memset(&sa_in,0,sizeof(sa_in));
			memcpy(&sa_in.sin_addr,rp->h_addr,rp->h_length);
			sa_in.sin_family = rp->h_addrtype;   
			sa_in.sin_port = htons(REQUEST_PORT);
			//Display the host machine internet address
			cout << "Connecting to remote host:";
			cout << inet_ntoa(sa_in.sin_addr) << endl;

			string file,dir;
			std::string command;
			getline(cin, command);
			do
			{

				cout << endl
					<< "Please select the correspoding functions" << endl
					<< "----------------------------------------" << endl
					<< "1 List file from Client" << endl
					<< "2 List file from Server " << endl
					<< "3 Get file from Server" << endl
					<< "4 Put file to Server " << endl
					<< "5 Delete files on Client " << endl
					<< "6 Delete files on Server" << endl
					<< "7 Quit " << endl
					<< "----------------------------------------" << endl;
				std::getline(std::cin, command);

				if (command == "1")
				{
					//List client files
					listClientFiles();
				}
				else if (command == "2")
				{
					//List server files.					
					dir = "list";
					if (threeWayHandShake() == true)
					{
						sendControl(file, dir);
						listServerFiles();
					}
				}
				else if (command == "3")
				{
					cout << "Type name of file: " << flush;
					cin >> file;
					dir = "get";
					if (threeWayHandShake() == true)
					{
						sendControl(file, dir);
						receiveFile(file);
					}
					getline(cin, command);

				}
				else if (command == "4")
				{
					cout << "Type name of file: " << flush;
					cin >> file;
					dir = "put";
					if (threeWayHandShake() == true)
					{
						sendControl(file, dir);
						sendFile(file);
					}
					getline(cin, command);

				}
				else if (command == "5")
				{
					//delete files on the client
					listClientFiles();
					deleteClientFiles();
					break;
				}
				else if (command == "6")
				{
					//delete files on the server
					cout << "Type name of file: " << flush;
					cin >> file;
					dir = "delete";
					if (threeWayHandShake() == true)
					{
						sendControl(file, dir);
						deleteServerFiles();
					}
					getline(cin, command);
					break;
				}
				else if (command == "7")
				{
					//Quit
					exit(0);
					break;
				}
				else
				{
					cout << endl << "A wrong option was chosen. Try again!" << endl
						<< "=============================" << endl << endl;
				}

			} while (true);
		}
	} // try loop
	//Display any needed error response.
	catch (char *str) { cerr<<str<<":"<<dec<<WSAGetLastError()<<endl;}
	//close the client socket
	fout.close();
	system("pause");
	closesocket(mySocket);
	/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
	WSACleanup();  
}
