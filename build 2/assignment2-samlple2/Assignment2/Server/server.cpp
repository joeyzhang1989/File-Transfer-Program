#include "server.h"
string Server::getFileSize(string filename){
	string size = "";
	string fullFilePath = filePath + filename;
	int fileSize = 0;
	stringstream out;
	ifstream infile(fullFilePath.c_str());
	if (infile.is_open())
	{
		infile.seekg(0, ios::end); // move to end of file
		fileSize = infile.tellg();
		out << fileSize;
		size = out.str();
	}
	else
	{
		cout << "could not open file\n";
	}
	return size;
}

void Server::receiveFile(string filename){
	int totalNumOfBytes = 0;
	string fullFilePath;
	if (TRACE)
	{
		fout << "Receiver: starting on host " << localhost << endl;
	}
	int num_of_file_packet = (atoi(cli_size.c_str()) / 512) + 1;
	FILE *stream;
	string ack;
	fullFilePath = filePath + filename;
	if ((stream = fopen(fullFilePath.c_str(), "wb")) != NULL)
	{
		if (TRACE)
		{
			fout << "Receiver: receiving file " << fullFilePath << endl;
		}
		segment_msg msgR;
		for (int i = 0; i<num_of_file_packet; i++)
		{
			do{
				memset(msgR.data, 0, 512);
				ibytesrecv = recvfrom(s, (char*)&msgR, sizeof(msgR), 0, (struct sockaddr *)&sa1, &client_length);
				totalNumOfBytes = totalNumOfBytes + ibytesrecv - sizeof(int);
				int seq = msgR.seqNo;
				if (TRACE)
				{
					fout << "Receiver: received packet " << seq << endl;
				}
				if (ibytesrecv>0)
				{
					stringstream out;
					current_Recv_Seq = msgR.seqNo;
					out << current_Recv_Seq;
					ack = "ACK" + out.str();
					memset(szbuffer, 0, 512);
					memcpy(szbuffer, ack.c_str(), 512);
					//if seqNo=next expected seqNo,write to file and send ack
					if (current_Recv_Seq == next_Recv_Seq)
					{
						fwrite((char*)msgR.data, sizeof(char), ibytesrecv - sizeof(int), stream);
						if (sendto(s, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa1, client_length) == -1)
							fprintf(stderr, "Error transmitting data.\n");
						totalNumOfBytes = totalNumOfBytes + ibytesrecv - sizeof(int);
						if (TRACE){
							fout << "Receiver: sent ACK for packet " << seq << endl;
						}
					}
					else
					{
						if (TRACE)
						{
							fout << "Receiver: received same packet " << seq << endl;
						}
						if (sendto(s, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa1, client_length) == -1)
							fprintf(stderr, "Error transmitting data.\n");
						if (TRACE)
						{
							fout << "Receiver: sent ACK for packet " << seq << endl;
						}
					}
				}
				else 
				{
					break;
				}
			} while (current_Recv_Seq == prev_Recv_Seq);

			prev_Recv_Seq = current_Recv_Seq;
			next_Recv_Seq = abs(next_Recv_Seq - 1);
		}
		cout << "Transfer complete \n";
		if (TRACE)
		{
			fout << "Sender: File transfer completed" << endl;
			fout << "Sender: Number of bytes received " << totalNumOfBytes << endl;
		}
	}
	else
	{
		cout << "Problem creating the file to be written\n";
	}
	fclose(stream);
	//system("pause");
}

void Server::sendFile(string filename){
	int totalNumOfPackets = 0;
	FILE *file;
	string fullFilePath;
	fullFilePath = filePath + filename;
	cout << "sending file" << fullFilePath << endl;
	int MAX_SIZE = 512;//maximum size of data to be read at a time
	file = fopen(fullFilePath.c_str(), "rb");
	if (TRACE)
	{
		fout << "Sender: starting on host " << localhost << endl;
	}
	if (file != NULL)
	{
		if (TRACE)
		{
			fout << "Sender: sending file " << fullFilePath << endl;
		}
		segment_msg msg;
		int num_of_file_packet = 0, num_of_read = 0;
		while (!feof(file))
		{
			num_of_file_packet = num_of_file_packet + 1;
			memset(msg.data, 0, 512);
			num_of_read = fread(msg.data, sizeof(char), MAX_SIZE, file);
			msg.seqNo = Send_Seq;
			// if it is "txt" file, display it on the screen.
			int lastPeriodPosition = filename.find_last_of('.');
			string extFileName = filename.substr(lastPeriodPosition + 1);
			//if (extFileName == "txt")
			//{
			//	cout << msg.data << endl;
			//}

			//send to receiver with currentSendSeqNo
			if (sendto(s, (char*)&msg, sizeof(msg), 0, (struct sockaddr *)&sa1, client_length) == -1)
				fprintf(stderr, "Error transmitting data.\n");
			if (TRACE)
			{
				fout << "Sender: sent packet " << Send_Seq << endl;
			}
			totalNumOfPackets++;
			//wait for ack.
			do{
				fd_set readfds; //fd_set is a type
				FD_ZERO(&readfds); //initialize
				FD_SET(s, &readfds); //put the socket in the set
				if (!(outfds = select(1, &readfds, NULL, NULL, &timeout)))
				{//timed out, resent                 
					if (sendto(s, (char*)&msg, sizeof(msg), 0, (struct sockaddr *)&sa1, client_length) == -1)
						fprintf(stderr, "Error transmitting data.\n");
					if (TRACE)
					{
						fout << "Sender: resent packet " << Send_Seq << endl;
					}
					totalNumOfPackets++;
				}
				if (outfds == 1)
				{//if not timed out, receive ack. store ack seq no
					memset(szbuffer, 0, 512);
					ibytesrecv = recvfrom(s, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa1, &client_length);
					if (TRACE)
					{
						fout << "Sender: received ACK for packet " << Send_Seq << endl;
					}
				}
			} while (outfds != 1);
			Send_Seq = abs(Send_Seq - 1);
		}
		cout << "File transfer completed\n";
		if (TRACE)
		{
			fout << "Sender: File transfer completed" << endl;
			fout << "Sender: Number of packets sent " << totalNumOfPackets << endl;
			fout << "Sender: Number of effective packets sent " << num_of_file_packet << endl;
		}
	}
	else
	{
		cout << "could not open " << file << " for reading\n";
	}
	fclose(file);
}

bool Server::threeWayHandShake(){
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
		fprintf(stderr, "Error transmitting data.\n");
	do{
		fd_set readfds; //fd_set is a type
		FD_ZERO(&readfds); //initialize
		FD_SET(s, &readfds); //put the socket in the set
		if (!(outfds = select(1, &readfds, NULL, NULL, &timeout)))
		{//timed out, return         
			if (sendto(s, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa1, client_length) == -1)
				fprintf(stderr, "Error transmitting data.\n");
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
					fprintf(stderr, "Error transmitting data.\n");
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

void Server::list()
{
	char endChar[5] = "\n";
	HANDLE hFind;
	WIN32_FIND_DATA data;
	segment_msg msg;

	wchar_t dirN[25] = _T(".\\*.*");
	//char *FileList = new char[BUFFER_SIZE];

	memset(msg.data, '\0', BUFFER_SIZE);
	hFind = FindFirstFile(dirN, &data);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			char fileN[256];
			size_t   i;

			wcstombs_s(&i, fileN, sizeof(data.cFileName), data.cFileName, sizeof(data.cFileName));

			if (strlen(msg.data) <= 0)
			{
				strcpy_s(msg.data, BUFFER_SIZE, fileN);//if there is nothing in FileList, add the first one into it.
			}
			else
			{
				// cout<<"Here3" <<endl;
				strcat_s(msg.data, BUFFER_SIZE, endChar);//endChar="\n". if there is a filename in the filelist, add a space line
				strcat_s(msg.data, BUFFER_SIZE, fileN);//then add the name behind the exits names.
			}
		} while (FindNextFile(hFind, &data));//find the next file after call the FindFirstFile

		FindClose(hFind);
	}
	msg.seqNo = Send_Seq;

	if (TRACE)
	{
		fout << "Sender: starting on host " << localhost << endl;
		fout << "Sender: sent the file list " << endl;
	}
	if (sendto(s, (char*)&msg, sizeof(msg), 0, (struct sockaddr *)&sa1, client_length) == -1)
		fprintf(stderr, "Error transmitting data.\n");
	if (TRACE)
	{
		fout << "Sender: sent packet " << Send_Seq << endl;
	}
	do{
		fd_set readfds; //fd_set is a type
		FD_ZERO(&readfds); //initialize
		FD_SET(s, &readfds); //put the socket in the set
		if (!(outfds = select(1, &readfds, NULL, NULL, &timeout)))
		{//timed out                   
			if (sendto(s, (char*)&msg, sizeof(msg), 0, (struct sockaddr *)&sa1, client_length) == -1)
				fprintf(stderr, "Error transmitting data.\n");
			if (TRACE)
			{
				fout << "Sender: resent packet " << Send_Seq << endl;
			}

		}
		if (outfds == 1)
		{
			memset(szbuffer, 0, 512);
			ibytesrecv = recvfrom(s, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa1, &client_length);
			if (TRACE)
			{
				fout << "Sender: received ACK for packet " << Send_Seq << endl;
			}
		}
	} while (outfds != 1);
	Send_Seq = abs(Send_Seq - 1);
}

void Server::receiveControl()
{
	segment_msg msgRC;
	string ack;
	string con_msg;
	string size;
	string re_msg;
	do{
		memset(msgRC.data, 0, 512);
		ibytesrecv = recvfrom(s, (char*)&msgRC, sizeof(msgRC), 0, (struct sockaddr *)&sa1, &client_length);
		int seq = msgRC.seqNo;
		if (ibytesrecv>0)
		{
			stringstream out;
			current_Recv_Seq = msgRC.seqNo;
			out << current_Recv_Seq;
			ack = "ACK" + out.str();
			//memset(szbuffer,0,512);
			//memcpy(szbuffer,ack.c_str(),512);
			// if seqNo=next expected seqNo
			if (current_Recv_Seq == next_Recv_Seq)
			{
				for (int i = 0; msgRC.data[i] != 0; i++)
					con_msg += msgRC.data[i];

				int pos = con_msg.find_first_of(',');
				cli_dir = con_msg.substr(0, pos);
				con_msg = con_msg.substr(pos + 1);

				pos = con_msg.find_first_of(',');
				cli_file = con_msg.substr(0, pos);
				con_msg = con_msg.substr(pos + 1);

				pos = con_msg.find_first_of(',');
				if (pos == string::npos)
				{
					cli_size = con_msg;
				}

				if (cli_dir == "get")
				{
					size = getFileSize(cli_file);
					re_msg = size + "," + ack;
				}
		
				memset(szbuffer, 0, 512);
				memcpy(szbuffer, re_msg.c_str(), 512);

				if (sendto(s, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa1, client_length) == -1)
					fprintf(stderr, "Error transmitting data.\n");
			}
			// else if seq number=previous seq number
			else
			{
				if (sendto(s, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa1, client_length) == -1)
					fprintf(stderr, "Error transmitting data.\n");
			}
		}
	} while (current_Recv_Seq == prev_Recv_Seq);
	prev_Recv_Seq = current_Recv_Seq;
	next_Recv_Seq = abs(next_Recv_Seq - 1);
}

void Server::deleteFiles(string fn)
{
	char filepath[FILELENGTH];
	char filename[FILELENGTH];
	char message[128];
	segment_msg msg;
	string file = fn;
	char *cstr = new char[file.length() + 1];
	strcpy(cstr, file.c_str());
	strcpy(filename, cstr);
	_getcwd(filepath, FILELENGTH);//return the currentpath to the buffer if fail will return false
	strcat(filepath, "\\");
	memset(msg.data, '\0', BUFFER_SIZE);
	if (!_access(filename, 00))//if file exist 
	{
		if (remove(filename) == -1)
		{
			if (TRACE)
			{
				fout << "Receiver: Could not delete : "<<filename <<"on"<< localhost << endl;
			}
			strcpy(message, " delete fail");
			strcat_s(filename,message);
			strcpy_s(msg.data, BUFFER_SIZE, filename);
		}
		else
		{
			if (TRACE)
			{
				fout << "delete :" << filename << "   successful" << endl;
			}
			strcpy(message, " delete successful");
			strcat_s(filename,message);
			strcpy_s(msg.data, BUFFER_SIZE, filename);
		}

	}
	else//file dose not exist
	{
		if (TRACE)
			{
				fout << filename << ":  file dose not exist" << endl;
			}
			strcpy(message, "file not exist");
			strcpy_s(msg.data, BUFFER_SIZE, message);
	}

	msg.seqNo = Send_Seq;
	if (sendto(s, (char*)&msg, sizeof(msg), 0, (struct sockaddr *)&sa1, client_length) == -1)
		fprintf(stderr, "Error transmitting data.\n");
	if (TRACE)
	{
		fout << "Sender: sent packet " << Send_Seq << endl;
	}
	do{
		fd_set readfds; //fd_set is a type
		FD_ZERO(&readfds); //initialize
		FD_SET(s, &readfds); //put the socket in the set
		if (!(outfds = select(1, &readfds, NULL, NULL, &timeout)))
		{//timed out                   
			if (sendto(s, (char*)&msg, sizeof(msg), 0, (struct sockaddr *)&sa1, client_length) == -1)
				fprintf(stderr, "Error transmitting data.\n");
			if (TRACE)
			{
				fout << "Sender: resent packet " << Send_Seq << endl;
			}

		}
		if (outfds == 1)
		{
			memset(szbuffer, 0, 512);
			ibytesrecv = recvfrom(s, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa1, &client_length);
			if (TRACE)
			{
				fout << "Sender: received ACK for packet " << Send_Seq << endl;
			}
		}
	} while (outfds != 1);
	Send_Seq = abs(Send_Seq - 1);
	delete[] cstr;
	
}
void Server::run()
{
	WSADATA wsadata;
	char *buffer;
	fout.open(fn);
	try
	{
		if (WSAStartup(0x0202, &wsadata) != 0)
		{
			cout << "Error in starting WSAStartup()\n";
		}
		else
		{
			buffer = "WSAStartup was suuccessful\n";
			WriteFile(test, buffer, sizeof(buffer), &dwtest, NULL);
			/* display the wsadata structure */
			cout << endl
				<< "==============The wsadata structure===================" << endl
				<< "wsadata.wVersion " << wsadata.wVersion << endl
				<< "wsadata.wHighVersion " << wsadata.wHighVersion << endl
				<< "wsadata.szDescription " << wsadata.szDescription << endl
				<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
				<< "wsadata.iMaxSockets " << wsadata.iMaxSockets << endl
				<< "wsadata.iMaxUdpDg " << wsadata.iMaxUdpDg << endl
				<< "======================================================" << endl;
		}
		//Display info of local host
		gethostname(localhost, 10);
		cout << "hostname: " << localhost << endl;
		if ((hp = gethostbyname(localhost)) == NULL)
		{
			cout << "gethostbyname() cannot get local host info?"
				<< WSAGetLastError() << endl;
			exit(1);
		}
		//Create the server socket
		if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
			throw "can't initialize socket";


		DWORD dwBytesReturned = 0;
		BOOL  bNewBehavior = FALSE;
		DWORD status;

		status = WSAIoctl(s, SIO_UDP_CONNRESET,
			&bNewBehavior,
			sizeof (bNewBehavior),
			NULL, 0, &dwBytesReturned,
			NULL, NULL);
		if (SOCKET_ERROR == status)
		{
			DWORD dwErr = WSAGetLastError();
			if (WSAEWOULDBLOCK == dwErr)
			{
				// nothing to do

			}
			else
			{
				printf("WSAIoctl(SIO_UDP_CONNRESET) Error: %d/n", dwErr);

			}
		}



		// For UDP protocol replace SOCK_STREAM with SOCK_DGRAM 
		//Fill-in Server Port and Address info.
		sa.sin_family = AF_INET;
		sa.sin_port = htons(SEND_PORT);
		sa.sin_addr.s_addr = htonl(INADDR_ANY);
		//Bind the server port
		if (bind(s, (LPSOCKADDR)&sa, sizeof(sa)) == SOCKET_ERROR)
			throw "can't bind the socket";
		cout << "Bind was successful" << endl;

		FD_ZERO(&readfds);
		//wait loop
		while (1)
		{
			client_length = (int)sizeof(struct sockaddr_in);
			timeout.tv_sec = SECT;
			timeout.tv_usec = USEC;
			FD_SET(s, &readfds);  //always check the listener
			if (!(outfds = select(infds, &readfds, NULL, NULL, tp))) {}
			else if (outfds == SOCKET_ERROR)
			{
				cout << "failure in Select";
				throw "failure in Select";
			}
			else if (FD_ISSET(s, &readfds))
				cout << "got a connection request" << endl;

			do{
				memset(szbuffer, 0, 512);
				ibytesrecv = recvfrom(s, szbuffer, sizeof(szbuffer), 0, (struct sockaddr *)&sa1, &client_length);
				if (ibytesrecv < 0)
				{
					fprintf(stderr, "Could not receive datagram.\n");
				}
				else
				{
					cout << szbuffer << endl;
				}
			} while (ibytesrecv <= 0);

			if (threeWayHandShake() == true)
			{
				receiveControl();
				if (cli_dir == "put")
				{
					receiveFile(cli_file);
				}
				else if (cli_dir == "list")
				{
					list();
				}
				else if (cli_dir == "get")
				{
					sendFile(cli_file);
				}
				else if (cli_dir == "delete")
				{
					deleteFiles(cli_file);
				}
			}

		}//wait loop
	} //try loop
	//Display needed error message.
	catch (char* str) { cerr << str << WSAGetLastError() << endl; }
	//close Client socket
	closesocket(s1);
	//close server socket
	closesocket(s);
	/* When done uninstall winsock.dll (WSACleanup()) and exit */
	WSACleanup();
	fout.close();
	system("pause");
}