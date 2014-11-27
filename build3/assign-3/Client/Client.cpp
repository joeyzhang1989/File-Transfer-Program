
#include "Client.h"


/*--------------------------*/
/* Method:			main()  */
/*--------------------------*/
using namespace std;

int main(void)
{
	client_info* req;
	req = (client_info*)client;
	WSADATA wsadata;
    
	try {

		if (WSAStartup(0x0202,&wsadata)!=0){  
			cout<<"Error in starting WSAStartup()" << endl;
		} 
		else 
		{
			buffer="WSAStartup was successful\n";   
            WriteFile(test,buffer,sizeof(buffer),&dwtest,NULL); 
		}  

		//Display name of local host.
		cout<<"\nFile Transfer Protocol Client" << endl;
		cout<<"================================================================" << endl;
		cout<<"" << endl;
		cout<<"" << endl;

		
		
		gethostname(localhost,21);
   		cout<<"\nClient program runs on host: \"" << localhost << "\"" << endl;

		if((hp=gethostbyname(localhost)) == NULL) 
			throw "gethostbyname failed\n";

		strcpy(req->localhost, localhost);
        
		while(1) {


			//Ask for name of remote server
			cout << "\nPlease type  remote host name: ";   
			cin >> remotehost ;
		  
			//Looking for remote host.if fail to find then throw exception

			if((rp=gethostbyname(remotehost)) == NULL) {
				throw "remote gethostbyname failed\n";
				return 0;
			}

			//Create the socket
			if((s = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==INVALID_SOCKET) 
				throw "Socket failed\n";

 
			//Specify server address for client to connect to server.
			memset(&serverAddr,0,sizeof(serverAddr));
			memcpy(&serverAddr.sin_addr,rp->h_addr,rp->h_length);
			serverAddr.sin_family = AF_INET;   
			serverAddr.sin_port = htons(ROUTER_PORT1);

			memset(&clientAddr,0,sizeof(struct sockaddr_in));
			clientAddr.sin_family = AF_INET;

		    clientAddr.sin_port = htons(PEER_PORT1);
         	clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);  

			//Bind the client port
			if (bind(s,(SOCKADDR *)&clientAddr,sizeof(clientAddr)) == SOCKET_ERROR)
				throw "can't bind the socket";
            
				//Prompts the user to enter the name of file he wish to transfer or get
			cout << "Type name of file to be transferred: ";
			cin >> req->filename;

			//Prompt the user for direction of transfer
			bool loop = false;
			do 
			{
				cout << "Type direction of transfer: ";
				cin >> direction;
			
				//Checking if the client wants to get or put a file.
				if ((strcmp(direction, "get") == 0) || (strcmp(direction, "put") ==0))
				{
					strcpy(req->direction, direction);
					loop = true;
				}
				else 
				{
					cout << "Incorrect direction command." << endl;//display error if command other than "get" or "put" is entered
					loop = false;
				}
			} while(loop == false); // Ask user to re-enter the direction if command was wrong


			//Get username.
		
		//	GetUserName(cusername, &dwusername);
			//strcpy(req->username, user);
		
			// Determine which direction of the transfer to be used
			if (strcmp(req->direction, "get") == 0) 
			{
				if (get(req->filename) == -1)
					throw "Unable to get file from server!\n";
				break;
			}
			else {
				if (put(req->filename) == -1)
					throw "Unable to transfer file to server!\n";
				break;
			}
		} //close while loop
		
	} // try loop

    //Display any needed error response.
	catch (char *str) { cerr<<str<<":"<<dec<<WSAGetLastError()<<endl;}

	//close the client socket
	closesocket(s);
         
	/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
	WSACleanup();  
    return 0;
}

/*----------------------------------------*/
/* 	 Send acknowledgement to server	      */
/*----------------------------------------*/
int sendACK() 
{
	bytessent = 0;

	if((bytessent = sendto(s, ack, sizeof(ack), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr))) == SOCKET_ERROR)	
	{
		cout << "Error in send in server program" << endl;
		return -1;
	}

	return 0;
}

/*----------------------------------------*/
/* receive acknowledgement from server	  */
/*----------------------------------------*/

int receiveACK() 
{
	bytessent = 0;

    bytesrecv = recvfrom(s, ack, sizeof(ack), 0, (struct sockaddr *)&clientAddr, &calen);

    if(bytesrecv < 0)      
	{
	    cout << "Error in receive in server program" << endl;
        return -1;
    }

    //verify the source of the response
    if(serverAddr.sin_addr.s_addr != clientAddr.sin_addr.s_addr)      
	{
        cout << "Error: reveiced a packet from unknown source.\n";
        exit(1);
    }

	return 0;
}

/*----------------------------------------*/
/* 		Function    - GET - 		      */
/*----------------------------------------*/					    
int get(char file[]) 
{
	FILE *stream;
	ofstream output(clogFile);

	bytessent = 0;
	int counter = 0;
	int lastSeq = 0;
	char lastWSeq;
	int finalPacket = 0;
	bool nakflag;

	bytessent = sendto(s, (char*)client, sizeof(client_info), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

	if (bytessent == SOCKET_ERROR) 
	{
		cout << "Sending request failed\n";
		return -1;
	}

    else
		cout << "\n\nSending request to \"" << remotehost << "\"..." <<endl;

	receiveFlag();
	
	if(flag[0]=='y') 
		cout << "Request is accepted by remote host." <<endl;
	
	else 
	{
		cout << "Remote host refused request!" << endl;
		return -1;
	}

	//Receive an answer from the server
	receiveFlag();

	//verify if there is an error on answer transfer 
	if((flag[0] != 'n') && (flag[0] != 'y')) 
	{
		cout<< "Error in data transfer."<<endl;
		return 0;
	}

	//file not found on server stop the transfer
	if(flag[0] == 'n')	
	{
		cout << endl << "Source file not found on remote host \""<< remotehost 
			<< "\". Transfer aborted!"<<endl;
		return 0;
	}

	//File found on server, continue.
	//Verify if the file exists in the client if yes ask client for overwrite permission. if no then continue transfer
	if((fopen(file, "r")) == NULL) 
	{
		if(fopen(file,"w") != NULL)	
		{
			flag[0] = 'y';
			sendFlag();
		}
	}

	//if file exsits ask if they want to overwrite
	else 
	{
		cout << "File already exist on local host. Do you want to overwrite? ('y' or 'n'): ";
		cin >> flag;
		cout << endl;

		//ask again if the user does not answer correctly
		while (strcmp(flag, "y")!=0 && strcmp(flag, "n")!=0)	
		{
			cout << "Please answer by 'y' or 'n': ";
			cin >> flag;
			cout << endl;
		}

		//Send the acknowlegment to the server
		sendFlag();
	}

	//Abort the transfer if the user type "n"
	if(flag[0] == 'n')	{
		cout << "\nTransfer aborted."<<endl;
		return 0;
	}

	//open file for writing
	if ((stream = fopen(file, "wb")) == NULL) 
	{
		cout<<"Cannot open file: " << file << endl;
		return -1;
	}

	//setting timer
	srand( (unsigned)time( NULL ) ); 
	ack_data* nack;
	nack = (ack_data*) ack ;
	packet_data* pack;
	pack = (packet_data*) packet;

	lastWSeq = SWS;
	nakflag = true;

	//wait for server reception response
	while (1) 
	{
		//clear the buffer
		clearbuf(pack->dataBuffer, dataSize);		
        ibytesrecv=0; 
		ibytesrecv = recvfrom(s,packet,sizeof(packet_data),0, (struct sockaddr *)&clientAddr, &calen);
        
		if (ibytesrecv == SOCKET_ERROR) 
			throw "Receive file failed\n";
		if(!sendwindow()) {

		char currentWSeq = lastWSeq + 1;

		// reset back to zero the window sequence
		if(currentWSeq > SWS) 
			currentWSeq = '0';
		
			//last dataBuffer size less than buffer size because negative
			if (pack->wSeq == currentWSeq && pack->seqNumber <= 0)
			{
				finalPacket = -pack->seqNumber;
				fwrite(pack->dataBuffer,sizeof(char),finalPacket,stream);		
				
				nack->type='a'; 
				nack->wSeq=pack->wSeq;
				nack->seqNumber = pack->seqNumber;

				ibytessent = 0;

				sendACK();
				
				cout<< "Receiver: received packet " << pack->seqNumber 
					<< " with window sequence "<<pack->wSeq<<"."<<endl;
				cout<< "Receiver: sent an ACK for packet " << pack->seqNumber 
					<< " with window sequence "<<pack->wSeq<<"."<<endl;
				
				output<< "Receiver: received packet " << pack->seqNumber 
				      << " with window sequence "<<pack->wSeq<<"."<<endl;
				
				output<< "Receiver: sent an ACK for packet " << pack->seqNumber 
					  << " with window sequence "<<pack->wSeq<<"."<<endl;
				
				output<< "\nTransfer of file from the client completed\n";
				
				cout << "\nTransfer of file from the client completed\n";
		
				fclose(stream);	
				output.close();
				return 0;
			}
			
			else if (pack->wSeq == currentWSeq && pack->seqNumber == lastSeq+1)
			{
				counter = 0;
				fwrite(pack->dataBuffer,sizeof(char),dataSize,stream);

				nack->type='a'; 
				nack->wSeq=pack->wSeq;
				nack->seqNumber=pack->seqNumber;
				
				ibytessent = 0;
				
				sendACK();

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
					nack->type='n';
					nack->wSeq=currentWSeq;
					nack->seqNumber=lastSeq+1;
					ibytessent = 0;
					sendACK();

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

/*----------------------------------------*/
/* Function			 - PUT - 		      */
/*----------------------------------------*/	
int put(char file[]) 
{

	FILE* stream;
	ofstream output(clogFile);

	int numofPackets, lastPacketSize;	
	int currentSeq = 0;
	int extraBytes = 0;	
	int lastACK = 0;
	int currentACK = 0;

	//the oldest window frame
	int start = 0;	
				
	//the newest window frame
	int end = 0;				
	int currentNAK = 0;
	char winSeq = '0';

	//int numread; 
	int	handle;
	int bytelen = dataSize;
	long filesize;
	timeout.tv_sec = 0;

	//tomeout value
	timeout.tv_usec = 300000;	
	bytessent = 0;

	int numofPacketsSent = 0;
	int effectiveBytesSent = 0;
	int bytesSent = 0;

	//Sending the first bytes
	bytessent = sendto(s, (char*)client, sizeof(client_info), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

	if (bytessent == SOCKET_ERROR) 
	{
		cout << "Sending request failed\n";
		return -1;
	}

    else
		cout << "\n\nSending request to \"" << remotehost << "\"..." <<endl;

	receiveFlag();

	if(flag[0]=='y') 
		cout << "Request is accepted by remote host." <<endl;

	else 
	{
		cout << "Remote host refused request!" << endl;
		return -1;
	}

	//Verify if the file exists on local host
	if ((fopen(file, "r")) ==NULL)	
	{
		flag[0] = 'n';

		//The file does not exists, stop the transfer
		sendFlag();

		cout <<  "\n Source file not found on local host \""<<localhost<<"\". Transfert aborted!" << endl;
		
		return -1;
	}

	else	
	{
		flag[0] = 'y';

		//The file does exist, continue the transfer
		sendFlag();
	}

	//wait for a response from the client to confirm the transfer
	receiveFlag();

	if(flag[0] == 'n')	
	{
		cout << "The file already exists on remote host. Do you want to overwrite? ('y' or 'n'): ";
		cin >> flag[0];
		cout << endl;

		//ask the question again if not answered correctly
		while (strcmp(flag, "y")!=0 && strcmp(flag, "n")!=0)	
		{
			cout << "Please answer by 'y' or 'n': ";
			cin >> flag[0];
			cout << endl;
		}
	}

	//Abort the transfer if the user answer n
	if(flag[0] == 'n')	
	{
		sendFlag();
		cout << "cancle the overwrite.\n";
		return -1;
	}

	else 
	{
		sendFlag();
	}		
	

	if ((stream = fopen(file, "rb")) == NULL) 
	{
		cout << "File does not exist or unable to open file!" << endl;
		return -1;
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
	
	ack_data* AckNak;
	AckNak = (ack_data*) ack;
	
	numofPackets = (int)(filesize/dataSize) + 1;
	cout << "Number of packets needed is " << numofPackets << "." << endl;
	
	lastPacketSize = filesize % dataSize;	
	
	
	cout << "waiting for transfer to complete..." << endl;
	
	//set up the first window
	cout<<"window size = "<<windowSize<<endl;

	for(int i=0; i<windowSize; i++) 
	{
		if(currentSeq < numofPackets) 
		{
			fread(pack->dataBuffer,sizeof(char),bytelen,stream);
			//bytesSent = bytesSent + packetSize;
			pack->wSeq = winSeq;

			currentSeq++;

			pack->seqNumber = currentSeq;

			if(currentSeq == numofPackets) 
				pack->seqNumber = -lastPacketSize;

			bytessent = 0;

			for(int j=0; j<sizeof(packet_data); j++)
				windowBuffer[i][j] = packet[j];

			cout<<"Sender: sent out packet "<<window[i]->seqNumber<<" with window sequence "<<window[i]->wSeq<<"."<<endl;
			output<<"Sender: sent out packet "<<window[i]->seqNumber<<" with window sequence "<<window[i]->wSeq<<"."<<endl;
			winSeq++;
			
			if(winSeq > SWS) 
				winSeq='0';
			
			end++; 
		}
	}
	
	end--;
	
	lastACK = end;

	currentACK = end;

	//Array populated and ready to send windows
	while(currentSeq < numofPackets) 
	{
		sendWindow(start, end);
		bool loop = true;

		while(loop == true) 
		{
			FD_SET(s, &readfds);

			//Send again if the sender times out.
			if(!(outfds=select(infds,&readfds,NULL,NULL,&timeout))) 
			{
				sendWindow(start, end);
				cout << "Sender: sent window again"<<endl;
				continue;
			}

			else if (outfds == SOCKET_ERROR) 
				throw "failure in Select";

			else if (FD_ISSET(s,&readfds))	
				loop = true;

			FD_CLR(s, &readfds);

			receiveACK();

			bool validACK = false;

			for(int k=0; k<windowSize; k++) 
			{
				if(window[k]->wSeq==AckNak->wSeq && window[k]->seqNumber==AckNak->seqNumber) 
				{
					validACK = true;
					break;

				}
			}if(!sendwindow()) {




			if(AckNak->type == 'a') 
			{	
				//if is an ACK 
				loop = true;


				if(validACK == true) 
				{
					currentACK = k;
					cout << "Sender: received an ACK for packet " << window[currentACK]->seqNumber 
						 << " with window sequence "<<window[currentACK]->wSeq<<"."<<endl;

					output<<"Sender: received an ACK for packet " << window[currentACK]->seqNumber 
						  <<" with window sequence "<<window[currentACK]->wSeq<<"."<<endl;
				  
					//SLIDING THE WINDOW
					start = lastACK + 1;
					
					if(start > windowSize-1) 
						start = 0;

					end = currentACK;

					lastACK = currentACK;

					int i = start;

					//Populating the window buffer
					while(i != end) 
					{
						if(currentSeq < numofPackets) 
						{
							fread(pack->dataBuffer,sizeof(char),bytelen,stream);
							pack->wSeq = winSeq;
							currentSeq++;	
							pack->seqNumber = currentSeq;

							if(currentSeq == numofPackets) 
								pack->seqNumber = -lastPacketSize;
							bytessent = 0;
							
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
					//populate the last window
					if(currentSeq < numofPackets) 
					{
						fread(pack->dataBuffer,sizeof(char),bytelen,stream);

						pack->wSeq = winSeq;
						currentSeq++;
						pack->seqNumber = currentSeq;
						
						if(currentSeq == numofPackets) 
							pack->seqNumber = -lastPacketSize;
						
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

				else if(AckNak->type == 'n') 
				{
					loop = true;

					if(validACK == true) 
					{
						currentNAK = k;
						start = currentNAK;
						end = lastACK;

						cout << "Sender: received an NAK for packet " << window[currentNAK]->seqNumber 
							 << " with window sequence "<<window[currentNAK]->wSeq<<"."<<endl;
					
						output<< "Sender: received an NAK for packet " << window[currentNAK]->seqNumber 
							  << " with window sequence "<<window[currentNAK]->wSeq<<"."<<endl;
						
						loop = true;
					}
				}
			}

			else 
			{
				if(AckNak->type == 'a' && validACK == true) 
				{
					cout << "Sender: dropped an ACK for packet " << window[k]->seqNumber 
						 << " with window sequence "<<window[k]->wSeq<<"."<<endl;

					output<<"Sender: dropped an ACK for packet " << window[k]->seqNumber 
						  <<" with window sequence "<<window[k]->wSeq<<"."<<endl;
				}
				
				else if(AckNak->type == 'n' && validACK == true) {

					cout << "Sender: dropped an NAK for packet " << window[k]->seqNumber 
					     << " with window sequence "<<window[k]->wSeq<<"."<<endl;
					
					output<<"Sender: dropped an NAK for packet " << window[k]->seqNumber 
						  <<" with window sequence "<<window[k]->wSeq<<"."<<endl;
				} 

				loop = true;
			}
		}
	}

    //Send the next window separately	
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
			cout << "Sender: sent window again"<<endl;
			continue;
		}

		else if (outfds == SOCKET_ERROR) 
			throw "failure in Select";

		else if (FD_ISSET(s,&readfds))	
			loop = true;

		FD_CLR(s, &readfds);
		receiveACK();
		
		bool validACK = false;
		for(int k=0; k<windowSize; k++) 
		{
			if(window[k]->wSeq==AckNak->wSeq && window[k]->seqNumber==AckNak->seqNumber) 
			{
				validACK = true;
				break;
			}
		}if(!sendwindow()){







		if(AckNak->type == 'a') 
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
				
				effectiveBytesSent = effectiveBytesSent + packetSize;
				loop = true;
				
			}
				
			if(AckNak->wSeq==window[lastACK]->wSeq && AckNak->seqNumber==window[lastACK]->seqNumber) 
			{
					cout << "File transfer completed successfully!"<<endl;
					output << "File transfer completed successfully!"<<endl;

					effectiveBytesSent = effectiveBytesSent - packetSize + lastPacketSize;
	
					//Writing to the log file.
					output << "\nSender: file transfer completed. " << endl;
					output << "Sender: number of effective bytes sent: " << effectiveBytesSent << "." << endl;
					output << "Sender: number of packets sent: " << numofPacketsSent << endl;
					output << "Sender: number of bytes sent: " << bytesSent + effectiveBytesSent << "." << endl;

					fclose(stream);
					output.close();
					
					return 0;
				
			}
		
			else if(AckNak->type == 'n') 
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
			if(AckNak->type == 'a' && validACK == true) 
			{
				
				cout<< "Sender: last window dropped an ACK for packet " << window[k]->seqNumber 
					<< " with window sequence "<<window[k]->wSeq<<"."<<endl;
				
				output<< "Sender: last window dropped an ACK for packet " << window[k]->seqNumber 
					  << " with window sequence "<<window[k]->wSeq<<"."<<endl;
			}
			
			else if(AckNak->type == 'n' && validACK == true) 
			{
				cout<< "Sender: last window dropped an NAK for packet " << window[k]->seqNumber 
				   	<< " with window sequence "<<window[k]->wSeq<<"."<<endl;
				
				output<< "Sender: last window dropped an NAK for packet " << window[k]->seqNumber 
					  << " with window sequence "<<window[k]->wSeq<<"."<<endl;
			}

			loop = true;
		}
		//numofPackets--;
		//numofPacketsSent++;
	}



	fclose(stream);
	output.close();
	
	cout << "\nTransfer of file to server completed!!" << endl;
	
	return 0;
}





/*----------------------------------------*/
/* 	 send flag function         		  */
/*----------------------------------------*/


int sendFlag() 
{
	bytessent = 0;

	if((bytessent = sendto(s, flag, sizeof(flag), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) == SOCKET_ERROR)	
	{
		cout << "Error in send in server program" << endl;
		return -1;
	}

	return 0;
}

/*----------------------------------------*/
/* 	receice flag function         		  */
/*----------------------------------------*/
int receiveFlag() 
{
	bytessent = 0;

    bytesrecv = recvfrom(s, flag, sizeof(flag), 0, (struct sockaddr *)&clientAddr, &calen);

    if(bytesrecv < 0)      
	{
	    cout<<"Error in receive in server program" << endl;
        return -1;
    }

    
    if(serverAddr.sin_addr.s_addr != clientAddr.sin_addr.s_addr)      
	{
        cout << "Error: reveiced a packet from unknown source.\n";
        exit(1);
    }

	return 0;
}

/*----------------------------------------*/
/* 	send window function         		  */
/*----------------------------------------*/
bool sendwindow() 
{
	if((rand() % 100) < r)
		return true;

	else
		return false;
}

void sendWindow(int head, int tail) 
{
	int i = head;

	while(i != tail) 
	{
		bytessent = 0;
		bytessent = sendto(s, windowBuffer[i], sizeof(packet_data), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
		
		if((bytessent == SOCKET_ERROR))
			throw "Error in send in server program\n";
		
		i++;
		
		if(i > windowSize-1) 
			i = 0;
	}
	
	bytessent = 0;
	bytessent = sendto(s, windowBuffer[i], sizeof(packet_data), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

	if((bytessent == SOCKET_ERROR))
		throw "Error in send in server program\n";
}



/*----------------------------------------*/
/* 	 function      getPacketLength   	  */
/*----------------------------------------*/


int getPacketLength(char* buffer) 
{
	int i = 0;					
	char c = buffer[i];

	while(c != NULL)
	{
		i++;
		c = buffer[i];
	}

	cout << "packet length is " << i<<endl;

	return i;
}


