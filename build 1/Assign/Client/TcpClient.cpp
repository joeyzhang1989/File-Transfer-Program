#include "TcpClient.h"

using namespace std;
//===========================================================
//Connect to a server.
//===========================================================
void TcpClient::connectServer(char remotehost[])
{
	try {
		struct hostent *p;
		if((p = gethostbyname(remotehost))==NULL)
		{
			printf("Host not exist:%s\n",remotehost);
			exit(0);
		}
	struct in_addr in;
    memcpy(&in,p->h_addr_list[0],sizeof(struct in_addr));
	ServPort=REQUEST_PORT;// assign the port to the server
	memset(&ServAddr, 0, sizeof(ServAddr));     /* Zero out structure */
	ServAddr.sin_family = AF_INET;             /* Internet address family */
	ServAddr.sin_addr.s_addr = *((unsigned long *) p->h_addr_list[0]);//ResolveName(servername);   /* Server IP address */
	ServAddr.sin_port   = htons(ServPort); /* Server port */
	
			//Create the socket
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) //create the socket 
		{
			throw "Socket Creating Error";
			exit (0);
		}

	if (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
		{
			throw "Socket Creating Error\n";
			exit (0);
		}
	else
		cout<<"connect to server"<<remotehost<<inet_ntoa(in)<<endl;

	}//try loop

	catch (char *str) {cerr<<str<<":"<<dec<<WSAGetLastError<<endl;}// catch the error messages throwed out

}


//===========================================================
//Get the file list from the client.
//===========================================================
void TcpClient::listClientFiles()
 {
	//pre-defined in the <io.h>
	//header The _findfirst function provides information about the first instance of a file name
	//that matches the file specified in the filespec argument
	struct _finddata_t files;
    long fileHandle;
	//char endChar[5]="\n";
	char dirbuffer[FILENAME_LENGTH];
	getcwd(dirbuffer,FILENAME_LENGTH);// return the currentpath to the buffer if fail will return false
	string curPath = strcat( dirbuffer,"\\*.*");//Append characters to string 

	if (!(fileHandle = _findfirst(curPath.c_str(), &files))) //see if there is file in the current directory
    {
        cout<<"the file not exist in the current directory"<<endl;
		return;
    }    
	else 
	{
		cout<<"=============The files on client====================="<<endl;
		
    do {
		if (_A_ARCH == files.attrib) //if it is archieved files print the names
            {
				cout<<files.name<<endl;
			}
	} while (!(_findnext(fileHandle, &files)));
		cout<<"====================================================="<<endl<<endl;
	}
	_findclose(fileHandle);// fileHandle equals to 0 means that there is no matching files exist
}
//===========================================================
//delete the files on the  client.
//===========================================================

void TcpClient::deleteClientFiles()

 {
	char filename[FILENAME_LENGTH];
	char filepath[FILENAME_LENGTH];
	getcwd(filepath,FILENAME_LENGTH);//return the currentpath to the buffer if fail will return false
	strcat(filepath,"\\");
	cout<<"enter the name of file that you want to delete:"<<endl;
	cin >> filename;//enter file name
	//strcpy(filename,strcat(filepath,filename));

	if(!_access(filename,00))//if file exist 
	{
			if( remove( filename) == -1 )
				 {
					 cout<< "Could not delete :"<<filename<<endl;
				 }
			else
				{
				cout<< "delete :"<<filename<<"   successful"<<endl;
				}

	}
	else//file dose not exist
	{
		cout<<filename<<":  file dose not exist"<<endl;
	}
}


//===========================================================
//put the file to the Server.
//===========================================================

void TcpClient::PutFileToServer()
{
	ClientFile clientfile;
	long size = 0;
	long sizeleft = 0;
	struct _stat putfilestate;
	char putfilename[FILENAME_LENGTH];
	char putfilebuffer[BUFFER_LENGTH];
	char head[20];//send the head infomation to the server to handle the function information
		strcpy(head,"PUT\r"); //send the put request to the head buffer send as a message to server
		int ibytesput = send(sock,head,20,0);
		if (ibytesput == SOCKET_ERROR)
			{
				cout<< "Send failed\n";  
		
			}
		else
			cout << "Message to server: " << head;
		int ibytesrecv=0; 
		if((ibytesrecv = recv(sock,head,20,0)) == SOCKET_ERROR)
			{
				cout<<"Receive failed\n";
	
			}
		else
		{
			cout << "Operation: " << head;  
			
		}
	cout<<endl
		<<"Please input the file name:"<<endl;
	cin>>putfilename;

	//cast it to the request packet structure		
	strcpy(clientfile.filename,putfilename);

	if( _stat(putfilename,&putfilestate)!=0)
		{
			printf("No such a file :%s\n",putfilename);
		}		
		else 
		{
			clientfile.filelenth=(long)putfilestate.st_size;		
			//contruct the SUCCESS response and send it out
			FILE *fp;
			fp = fopen(clientfile.filename, "rb");
			
			
			send(sock, (char *)&clientfile, sizeof(clientfile), 0);// send the file information 
			//to the server allow server to set corresponding memory space
			long sizeleft = (long) putfilestate.st_size; // size to send while buffer is less than sizeleft
			
			//ifstream file (putfilename, std::ifstream::binary);
			//if (file.is_open())// check if the file can be opened
			//{
				//file.seekg(0, file.end);
				//size = (long)file.tellg();
				//sizeleft = size;
				//clientfile.filelenth = size;
				//file.seekg(0, file.end);
			while (sizeleft >= 0)
			{
				if (sizeleft > BUFFER_LENGTH)
				{
					int byteread = fread(&putfilebuffer, 1, BUFFER_LENGTH, fp);
					send(sock, putfilebuffer, BUFFER_LENGTH, 0);
					sizeleft = sizeleft - BUFFER_LENGTH;
				}
				else
				{
					fread(&putfilebuffer, 1, sizeleft, fp);
					send(sock, putfilebuffer, sizeleft, 0);
					sizeleft = sizeleft - BUFFER_LENGTH;
					fclose(fp);
				}
			}cout << "Upload Finished" << endl;

	}
}

//===========================================================
//list the files on Server.
//===========================================================

void TcpClient::listServerFiles()

 {
	char listbuffer[FILENAME_LENGTH];
	int ibufferlen=0;
	int ibytessent=0;
	int ibytesrecv=0;
	
	sprintf(listbuffer,"LIST\r"); 

		ibytessent=0;    
		ibufferlen = strlen(listbuffer);
		ibytessent = send(sock,listbuffer,ibufferlen,0);
		if (ibytessent == SOCKET_ERROR)
			throw "Send failed\n";  
		else
			cout << "Message to server: " << listbuffer<<endl;
		printf("Server File List\n");
		printf("********************************\n");
		listbuffer[0] = '\0';
		do
		{
			if(listbuffer[0] == '\0');
			else
			cout << "fileName: " << listbuffer<<endl;  
			if((ibytesrecv = recv(sock,listbuffer,sizeof(listbuffer),0)) == SOCKET_ERROR)
			throw "Receive failed\n";
				
		} while (listbuffer[0] != '@');
	 	printf("********************************\n");
}

//===========================================================
//get file from the Server.
//===========================================================
void TcpClient::GetFileFromServer()
{
	char getfilename[FILENAME_LENGTH];
	ClientFile clientfilename;
	char recvbuffer[BUFFER_LENGTH];
	int ibufferlen;
	int ibytessent=0;    
	int resu=0;
	struct _stat filestate;
	char head[128];//send the head infomation to the server to handle the function information
		strcpy(head,"GET\r"); //send the get request to the head buffer send as a message to server
		ibytessent  = send(sock,head,BUFFER_LENGTH,0);
		if (ibytessent  == SOCKET_ERROR)
			{
				cout<< "Send failed\n";  
		
			}
		else
			cout << "Message to server: " << head;
		int ibytesrecv=0; 
		if((ibytesrecv = recv(sock,head,BUFFER_LENGTH,0)) == SOCKET_ERROR)
			{
				cout<<"Receive failed\n";
	
			}
		else
		{
			cout << "Operation: " << head;  
			
		}
		cout<<endl
			<<"Please input the file name:"<<endl;
		cin>>getfilename;

		ibytessent=0;    
		ibufferlen = sizeof(getfilename);
		ibytessent = send(sock,getfilename,ibufferlen,0);
		if (ibytessent == SOCKET_ERROR)
			throw "Send failed\n";  
		else
			cout << "Get file from server: " << getfilename<<endl;;

		//receive the response
		recv(sock,(char *)&clientfilename,sizeof(clientfilename),0);
			

		//check if file exist on server, compare the names of filename and the response in respp structure
		int ms = strcmp(getfilename, clientfilename.filename);
		if(ms == 0)
		{
			printf("Response:file size %ld\n",clientfilename.filelenth);
			// get the filesize
			long fsize = (long)clientfilename.filelenth;
			if((resu = _stat(getfilename,&filestate))==0)
				printf("Over Write local file:%s\n",getfilename);
			// start getting the file
			int receivesize =0; // number of received bytes
			FILE *fp;
			fp = fopen (clientfilename.filename,"wb");
	
			cout << "Size of file to be received " << fsize << endl;
			
			while (fsize > 0)
			{
				if (fsize > BUFFER_LENGTH)
				{
					receivesize = recv(sock, recvbuffer, BUFFER_LENGTH, 0);
					fwrite(recvbuffer, 1, receivesize, fp);
				}
				else
				{
					receivesize = recv(sock, recvbuffer, fsize, 0);
					fwrite(recvbuffer, 1, receivesize, fp);
				}

				fsize = fsize - receivesize;

			}
			fclose(fp);
			cout << "Download Finished" << endl;
		}
		else
		{
			printf("No such file on the server");
		}
}
//===========================================================
//delete the files on the Server.
//===========================================================
void TcpClient::deleteServerFiles()
{
	char filename[FILENAME_LENGTH];
	char head[BUFFER_LENGTH];//send the head infomation to the server to handle the function information
	ClientFile filenames;
	struct _stat filesstate;
	strcpy(head, "DELETE\r"); //send the delete request to the head buffer send as a message to server
	int ibytesput = send(sock, head, BUFFER_LENGTH, 0);
	if (ibytesput == SOCKET_ERROR)
	{
		cout << "Send failed\n";

	}
	else
		cout << "Message to server: " << head;
	int ibytesrecv = 0;
	if ((ibytesrecv = recv(sock, head, BUFFER_LENGTH, 0)) == SOCKET_ERROR)
	{
		cout << "Receive failed\n";

	}
	else
	{
		cout << "Operation: " << head;

	}
	cout << endl
		<< "Please input the file name:" << endl;
	cin >> filename;
	strcpy(filenames.filename, filename);
	send(sock, (char *)&filenames, sizeof(filenames), 0);
	//send(sock, filename, FILENAME_LENGTH, 0);

	//receive the response
	recv(sock, (char *)&filenames, sizeof(filenames), 0);

	//check if file exist on server, compare the names of filename and the response in respp structure
	int ms = strcmp(filename, filenames.filename);
	if (ms == 0)
	{
		// send the confirm info of the file you will delete on server
		cout << "the file you will be deleted on server is " << filenames.filename << endl;
		cout << "Delete is Finished" << endl;

	}
	else
	{
		recv(sock, filename, FILENAME_LENGTH, 0);
		cout << "the file you  want delete on Server not exist check on your input";
	}


}
//===========================================================
//close the socket
//===========================================================
void TcpClient::closeconnect()
{
	closesocket(sock);
}


TcpClient::~TcpClient()
{
	/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
	WSACleanup();  
}
