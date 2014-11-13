/* Client side of project to execuate the functions
related to the file transfer to the server 
*/
#include "StartClient.h"// header file 
#include "TcpClient.h"
using namespace std;
// the file information to be used for the transferring files  


void localhostInfomation()
{
	gethostname(localhost,20);
	if((localhostInfo=gethostbyname(localhost)) == NULL)
	{
		cout<<"Get localhost information failed!"<<endl;
		return;
	}
	cout<<endl
		<<"client starting at host:"<<localhostInfo->h_name<<endl;
	return ;

}

string_code option(std::string command)
{

	if (command == "1") return first;

	if (command == "2") return second;

	if (command == "3") return third;

	if (command == "4") return forth;

	if (command == "5") return fifth;

	if (command == "6") return sixth;

	if (command == "7") return seventh;

}

boolean checkRemotehostLength(char * filename)
{
	
	if (strlen(filename)>REMOTEHOST_LENGTH)
		{
			cout<<"the length of the remotehost exceed the maximum[10], please verify then re-enter"<<endl;
			return false;
		}	
	else 
	{
		return true;
	}
}
int main()
{
	TcpClient *tc=new TcpClient();
	char remotehost[REMOTEHOST_LENGTH];
	char command[COMMAND_LENGTH];
	WSADATA wsadata;
	if (WSAStartup(0x0202,&wsadata)!=0)
	{  
		WSACleanup();  
		cout<< "Error in starting WSAStartup()\n"<<endl;
	}
	localhostInfomation();
	
	do
	{
		// obtain the name of server
		cout << "Please input the name of the romotehost you want to connect: "<<endl;
		cin >> remotehost;// type the romotehost name
	}while((romotehostEntity=gethostbyname(remotehost)) == NULL);
	// check the remotehost lengh 
	if (checkRemotehostLength(remotehost)==false)
	{
		cin>>remotehost;
		//checkRemotehostLength(remotehost);
	}
	do
      {
	  cout<<endl
			<< "Please select the correspoding functions"<< endl
			<< "----------------------------------------"<< endl
			<< "1 List file from Client"				 << endl
			<< "2 List file from Server "				 << endl
			<< "3 Get file from Server"					 << endl
			<< "4 Put file to Server "					 << endl
			<< "5 Delete files on Client "				 << endl
			<< "6 Quit "								 << endl
			<< "7 Delete files on Server"				 << endl
			<< "----------------------------------------"<< endl;
	  cin>>command;//enter the command
	  switch (option(command))
	  {
		case first:// list files from client 
			{
				tc->listClientFiles();
			    break;
			}
		case second: //list files from server
			{
				tc->connectServer(remotehost);
				tc->listServerFiles();
				break;
			}
		case third:// get files from the server
			{
				tc->connectServer(remotehost);
				tc->listServerFiles();
				tc->closeconnect();
				tc->connectServer(remotehost);
				tc->GetFileFromServer();
				break;
			}
		case forth://put the files to server
			{
				tc->listClientFiles();
				tc->connectServer(remotehost);
				tc->PutFileToServer();
				break;
			}
		case fifth: // delete the files on client
			{
				tc->deleteClientFiles();
				break;
			}
		case sixth:// close the client
			{
				exit(0);
				break;
			}
		case seventh:// delete files on the server
		{
				tc->connectServer(remotehost);
				tc->listServerFiles();
				tc->closeconnect();
				tc->connectServer(remotehost);
				tc->deleteServerFiles();
				break;

		}
		default:
			{
				cout<< "invalid input,Please verify your input"<<endl;
			}
	    }
    } while (true);// the loop continue for user to keep seeing the menu
	
  return 0;// end of the program 
}
