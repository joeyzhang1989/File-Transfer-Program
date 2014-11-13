//    SERVER TCP PROGRAM
// revised and tidied up by
// J.W. Atwood
// 1999 June 30
// There is still some leftover trash in this code.

/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#include "Server.h"
void * Thread::pthread_callback (void * ptrThis) 
{
    if ( ptrThis == NULL )
	     return NULL ;	
    Thread  * ptr_this =(Thread *)(ptrThis) ;
    ptr_this->run();
    return NULL;
} 

void Thread::start () 
{
	int threadresul;
    if(( threadresul = _beginthread((void (*)(void *))Thread::pthread_callback,STKSIZE,this ))<0)
	{
		printf("_beginthread error\n");
		exit(-1);
	}
	else
	{
		printf("Thread:%d creat ok\n",num);
		num++;
	}
	
}

	
	void TcpThread::run()
	{
				memset(szbuffer,0,sizeof(szbuffer));
				//Fill in szbuffer from accepted request.
				if((ibytesrecv = recv(s1,szbuffer,128,0)) == SOCKET_ERROR)
					throw "Receive error in server program\n";
				//Print reciept of successful message. 
				cout << "This is message from client: " << szbuffer << endl;		
				if(strcmp(szbuffer,"LIST\r")==0)
					listFiles(s1);

				if(strcmp(szbuffer,"GET\r")==0)
				{
					if((ibytessent = send(s1,szbuffer,128,0))==SOCKET_ERROR)
						throw "error in send in server program\n";
					else cout << "Operation:" << szbuffer << endl;  
					PutFileToClient(s1);
				}

				if(strcmp(szbuffer,"PUT\r")==0)
				{
					if((ibytessent = send(s1,szbuffer,128,0))==SOCKET_ERROR)
						throw "error in send in server program\n";
					else cout << "Operation:" << szbuffer << endl;  
					GetFileFromClient(s1);
				}
				if (strcmp(szbuffer, "DELETE\r") == 0)
				{
					if ((ibytessent = send(s1, szbuffer, 128, 0)) == SOCKET_ERROR)
						throw "error in send in server program\n";
					else cout << "Operation:" << szbuffer << endl;
					DeleteServerFiles(s1);
				}
	}

	void TcpServer::start()
	{
			while(1)
			{
				FD_SET(s,&readfds);  //always check the listener
				if(!(outfds=select(infds,&readfds,NULL,NULL,tp))) {}
				else if (outfds == SOCKET_ERROR) throw "failure in Select";
				else if (FD_ISSET(s,&readfds))  cout << "got a connection request" << endl; 
				//Found a connection request, try to accept. 
				if((s1=accept(s,&ca.generic,&calen))==INVALID_SOCKET)
					throw "Couldn't accept connection\n";
				//Connection request accepted.
				cout<<"accepted connection from "<<inet_ntoa(ca.ca_in.sin_addr)<<":"
					<<hex<<htons(ca.ca_in.sin_port)<<endl;
				TcpThread *pt=new TcpThread(s1);
				pt->start();
			}
	}

	TcpServer::TcpServer()
	{
		port = REQUEST_PORT;
		WSADATA wsadata;
    		 
			if (WSAStartup(0x0202,&wsadata)!=0){  
				cout<<"Error in starting WSAStartup()\n";
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
			if((s = socket(AF_INET,SOCK_STREAM,0))==INVALID_SOCKET) 
				throw "can't initialize socket";
			// For UDP protocol replace SOCK_STREAM with SOCK_DGRAM 


			//Fill-in Server Port and Address info.
			sa.sin_family = AF_INET;
			sa.sin_port = htons(port);
			sa.sin_addr.s_addr = htonl(INADDR_ANY);


			//Bind the server port

			if(bind(s,(LPSOCKADDR)&sa,sizeof(sa))==SOCKET_ERROR)
				throw "can't bind the socket";
			cout << "Bind was successful" << endl;

			//Successfull bind, now listen for client requests.

			if(listen(s,10) == SOCKET_ERROR)
				throw "couldn't  set up listen on socket";
			else cout << "Listen was successful" << endl;

			FD_ZERO(&readfds);
	}

	TcpServer::~TcpServer()
	{
	WSACleanup();
	}

	void TcpThread::GetFileFromClient(SOCKET gs1)
	{
		char getclientfilename[260];
		Fileinfo clientputfilename;
		char serverrecvbuffer[BUFFER_LENGTH];
		char *buffer;
		struct _stat getfilestate;

		memset(getclientfilename,0,sizeof(getclientfilename));
		//receive the response
		recv(gs1,(char *)&clientputfilename,sizeof(clientputfilename),0);
			

		//cast it to the response structure
		printf("Response:file size %ld\n",clientputfilename.filelenth);
		
		//check if file exist on server, compare the names of filename and the response in respp structure
		if( _stat(clientputfilename.filename,&getfilestate)==0)
			printf("Over write file on Server:%s\n",clientputfilename.filename);
			// get the filesize
		
			long fsize = (long)clientputfilename.filelenth;
	
			// start getting the file
			int receivesize =0; // number of received bytes
			//FILE *fp;
			//fp = fopen (clientputfilename.filename,"wb");
			
			FILE *fp;
			fp = fopen(clientputfilename.filename, "wb");

			cout << "Size of file to be received " << fsize << endl;

			while (fsize > 0)
			{
				if (fsize > BUFFER_LENGTH)
				{
					receivesize = recv(gs1, serverrecvbuffer, BUFFER_LENGTH, 0);
					fwrite(serverrecvbuffer, 1, receivesize, fp);
				}
				else
				{
					receivesize = recv(gs1, serverrecvbuffer, fsize, 0);
					fwrite(serverrecvbuffer, 1, receivesize, fp);
				}

				fsize = fsize - receivesize;

			}
			fclose(fp);
			cout << "Download Finished" << endl;
			closesocket(gs1);
		/*	while (fsize > 0)
			{
				if (fsize > BUFFER_LENGTH)
				{
					receivesize = recv(gs1,serverrecvbuffer,BUFFER_LENGTH,0);
					ofstream outfile(clientputfilename.filename, ios::binary | ios::app);
					buffer = serverrecvbuffer;
					outfile.write(buffer,BUFFER_LENGTH);
					outfile.close();
					outfile.clear();
					//fwrite(serverrecvbuffer,1,receivesize,fp);
				}
				else
				{
					receivesize = recv(gs1,serverrecvbuffer,fsize,0);
					ofstream outfile(clientputfilename.filename, ios::binary | ios::app);
					buffer = serverrecvbuffer;
					outfile.write(buffer, receivesize);
					outfile.close();
					outfile.clear();
					//fwrite(serverrecvbuffer, 1, fsize, fp); 
				}
	
				fsize = fsize - receivesize;
	
			}
			//fclose(fp);
			cout << "Download Finished" << endl;
			closesocket(gs1);
			*/
	}

	void TcpThread::PutFileToClient(SOCKET ps1)
	{
		Fileinfo serverfile;

		struct _stat stat_buf;
		int result;
		char filenamebuffer[260];
		char filebuffer[BUFFER_LENGTH];

		memset(filenamebuffer,0,sizeof(filenamebuffer));
		int i = recv(ps1,filenamebuffer,sizeof(filenamebuffer),0);
		int n = sizeof(filenamebuffer);
		if(i!=n)
			printf("Receive Req error,exit");
	
		//cast it to the request packet structure		
		strcpy(serverfile.filename,filenamebuffer);
		if((result = _stat(serverfile.filename,&stat_buf))!=0)
			{
				sprintf(filenamebuffer,"No such a file");
				if(send(ps1,filenamebuffer,sizeof(filenamebuffer),0)!=sizeof(filenamebuffer))
				printf("send error\n");

			}		
		else 
		{
			serverfile.filelenth=stat_buf.st_size;
			//contruct the SUCCESS response and send it out
			send(ps1,(char *)&serverfile,sizeof(serverfile),0)!=sizeof(serverfile);
	
			int sizeleft = (int) stat_buf.st_size; // size to send while buffer is less than sizeleft
			FILE *fp;
			fp = fopen (serverfile.filename,"rb");

			while (sizeleft >= 0)
			{
				if (sizeleft > BUFFER_LENGTH)
				{
					fread(&filebuffer, 1, BUFFER_LENGTH, fp);
					send(ps1, filebuffer, BUFFER_LENGTH, 0);
					sizeleft = sizeleft - BUFFER_LENGTH;
				}
				else
				{
					fread(&filebuffer, 1, sizeleft, fp);
					send(ps1, filebuffer, sizeleft, 0);
					sizeleft = sizeleft - BUFFER_LENGTH;
					fclose(fp);
				}
			}
			cout << "Upload Finished" << endl;
		}
		closesocket(ps1);
			/*ifstream file (filenamebuffer, std::ifstream::binary);
			if (file.is_open())// check if the file can be opened
			{
			while (sizeleft >0)
			{
				file.seekg (0, ios::beg);
				if (sizeleft > BUFFER_LENGTH)
				{
					file.read(filebuffer,BUFFER_LENGTH);
					send(ps1,filebuffer,BUFFER_LENGTH,0);	
					sizeleft = sizeleft - BUFFER_LENGTH;
				}
				else 
				{		
					file.read(filebuffer,sizeleft);
					send(ps1,filebuffer,sizeleft,0);
					sizeleft = 0;
					file.clear();
					file.close();
					fclose(fp);
				}
			}cout << "Upload Finished" << endl;

		}
		closesocket(ps1);
		}
		*/
	}

	void TcpThread::listFiles(SOCKET ss1)

 {
	struct _finddata_t serverfile;
    long serverfileHandle;
	char serverdirbuffer[256];
	getcwd(serverdirbuffer,256);

    //string curPath";
	string curPath = strcat( serverdirbuffer,"\\*.*");

    if ((serverfileHandle = _findfirst(curPath.c_str(), &serverfile)) == -1) 
    {
        return;
    }    
    do {
            if (_A_ARCH == serverfile.attrib) 
            {				
				if(send(ss1,serverfile.name,sizeof(serverfile.name),0) != sizeof(serverfile.name))
					printf("error in send in server program\n");
        }
    } while (!(_findnext(serverfileHandle, &serverfile)));
    _findclose(serverfileHandle);
				char servbuffer[260];
				servbuffer[0] = '@';
				if(send(ss1,servbuffer,sizeof(servbuffer),0) != sizeof(servbuffer))
					printf("error in send in server program\n");				
}
	//===========================================================
	//delete the files on the Server.
	//===========================================================
	void TcpThread::DeleteServerFiles(SOCKET ds1)
	{
		char filename[260];
		char filepath[260];
		Fileinfo filenames;
		struct _stat filesstate;
		memset(filename, 0, sizeof(filename));
		//receive the response
		recv(ds1,(char *) &filenames, sizeof(filenames), 0);
		getcwd(filepath, FILENAME_LENGTH);//return the currentpath to the buffer if fail will return false
		strcat(filepath, "\\");
			if (!_access(filenames.filename, 00))//if file exist 
			{
				if (remove(filenames.filename) == -1)
				{
					cout << "Could not delete :" << filenames.filename << endl;
				}
				else
				{
					cout << "delete :" << filenames.filename << "   successful" << endl;
					send(ds1, (char *)&filenames.filename
						, sizeof(filenames.filelenth), 0);
					closesocket(ds1);
				}

			}
			else//file dose not exist
			{
				cout << filename << ":  file dose not exist or filename is invalid" << endl;
				send(ds1, filename, 260, 0);
				closesocket(ds1);
			}
	
}

	int main(void){
		//start of the program

				TcpServer tserver;
				tserver.start();

		//close Client socket
		closesocket(s1);		

		//close server socket
		closesocket(s);

		/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
		WSACleanup();
		return 0;
	}




