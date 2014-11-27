#include "Client.h"

using namespace std;
int main ()
{
	run ();
	return 0;
}

int run (){
    srand ( time(NULL) );

    WSADATA wsadata;                                    // WSA connection
    FILE* logfile = fopen("client.log", "w");

    try {

        if (WSAStartup(0x0202,&wsadata)!=0){  
            throw "Error in starting WSAStartup";
        } else {

            /* Display the wsadata structure */
            cout<< endl
                << "wsadata.wVersion "       << wsadata.wVersion       << endl
                << "wsadata.wHighVersion "   << wsadata.wHighVersion   << endl
                << "wsadata.szDescription "  << wsadata.szDescription  << endl
                << "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
                << "wsadata.iMaxSockets "    << wsadata.iMaxSockets    << endl
                << "wsadata.iMaxUdpDg "      << wsadata.iMaxUdpDg      << endl;
        }  

        client_socket = open_port(PEER_PORT1);

        prompt("Enter the router hostname: ",router);
        sa_out = prepare_peer_connection(router, ROUTER_PORT1);

        prompt("Enter a filename: ",filename);                  // Retrieve a filename from the client
        prompt("Direction of transfer [get|put]: ",direction);  // Retrieve a transfer direction

        // Make sure the direction is one of get or put
        if(!strcmp(direction,GET) || !strcmp(direction,PUT)){ 

			//Get username.
		  
			GetUserName(cusername, &dwusername);

            // Retrieve the local user name
     

            int selected = rand() % 256;
            int received, verify;

            int client_num = 0; // Client packet number
            int server_num = 0; // Server packet number

            int progress = 0;
            int rcv;

            cout << "Starting packet ID negotiation..." << endl;

            while(1){

                // Send a random number to the server
                if(progress < 1){
                    memset(buffer, 0, sizeof(buffer));
                    sprintf(buffer,"RAND %d",selected);
                    cout << "Sending " << buffer << endl;
                    if((rcv = send_safe(client_socket, sa_out, buffer, RAWBUF_SIZE, 200)) == 201){
                        progress = 1;
                    }else if(rcv != 200){
                        continue;
                    }

                    // Finally wait for a response from the server with the number
                    if(recv_safe(client_socket, sa_out, buffer, RAWBUF_SIZE, 100) == 100){
                        cout << "Received " << buffer << endl;
                        sscanf(buffer,"RAND %d %d",&verify,&received);
                    }else continue;
                        progress = 1;
                }

                // Send acknowledgement to the server along with our random number
                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer,"RAND %d",received);
                cout << "Sending " << buffer << endl;
                if(send_safe(client_socket, sa_out, buffer, RAWBUF_SIZE, 201) != 201){
                    progress = 0;
                    continue;
                } 
                break;
            }

            client_num = selected % WINDOW_SIZE + 1;
            server_num = received % WINDOW_SIZE + 1;

            cout << "Negotiated server start " << server_num << " and client start " << client_num << endl;

            sprintf(trace_data, "Negotiated srv %d and cli %d", server_num, client_num);
            write_log(logfile, cusername, trace_data);
            // Send client headers
            sprintf(buffer,HEADER, cusername, direction, filename); 
            while((rcv = send_safe(client_socket,sa_out,buffer,RAWBUF_SIZE,777)) != 777){
                if(rcv == 101) break;
            }



            // Perform a get request
            if(!strcmp(direction,GET)){
                get(client_socket, sa_out, cusername, filename, client_num, server_num, logfile);
                
            }else if(!strcmp(direction,PUT)){
                put(client_socket, sa_out, cusername, filename, client_num, server_num, logfile);
            }

        }else{
            throw "The method you requested does not exist, use get or put";
        }

    } // try loop

    //Display any needed error response.
    catch (const char *str) { 
        cerr << str << WSAGetLastError() << endl;
    }

    //close the client socket and clean up
    fclose(logfile);
    closesocket(client_socket);
    WSACleanup();  
    return 0;
}


void prompt(const char* message, char*buffer){
    cout << message << flush ;  // Print the message
    cin >> buffer;              // Record the input into the buffer
}

/**
 * Open a port
 * Opens a local port for new connections
 */
SOCKET open_port(int port){

    // Retrieve the local hostname
    gethostname(hostname,21);

    if((hp=gethostbyname(hostname)) == NULL)   throw "Could not determine a host address from supplied name";
    //Fill-in UDP Port and Address info.
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
     // Create the socket
    if((sock = socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET) throw "Generating a new local socket failed";
    // Bind to the client port
    if (bind(sock,(LPSOCKADDR)&sa,sizeof(sa)) == SOCKET_ERROR) throw "Could not bind socket to supplied port";

    return sock;
}

/**
 * Prepare a peer connection
 * Generates a socket connection object to a peer
 */
SOCKADDR_IN prepare_peer_connection(char* hostname, int port){
    
    if((hp=gethostbyname(hostname)) == NULL) throw "Could not determine a host address from supplied name";

    cout << "Peer connection: " << hostname << ":" << port << endl;

    // Fill in port and address information
    memcpy(&sa.sin_addr,hp->h_addr,hp->h_length);
    sa.sin_family = hp->h_addrtype;   
    sa.sin_port = htons(port);
    return sa;
}


void write_log(FILE* logfile, char* username, char* message){
    fprintf(logfile, "%s > %s\n", username, message);
    memset(message, 0, sizeof(message));
}

/**
 * Create a packet with an identifier tag
 * Takes a buffer of size packet_size and generates a packet of size packet_size + sizeof(int)
 */
void make_packet(char* packet, char* buffer, int buffer_size, int number){
    memcpy(packet, buffer, buffer_size);    // Copy the base packet into the full packet
    memcpy(packet + buffer_size, &number, sizeof( int ) );  // Copy the tag into the packet
}

/**
 * Split a packet into packet name and identifier
 * Takes a packet of size packet_size + sizeof(int) and extracts a packet (size packet_size) and its integer identifier
 */
void split_packet(char* packet, char* buffer, int buffer_size, int* number){
    memcpy(buffer, packet, buffer_size);                // Extract the actual packet data
    memcpy(number, packet + buffer_size, sizeof(int));  // Extract the packet identifier
}

/**
 * Send Packet over a UDP socket
 * Generates a tagged packet and sends it over the supplied socket
 */
int send_packet(SOCKET sock, SOCKADDR_IN sa, char* buffer, int size, int pid){
    int ibytessent = 0;
    int packet_size = size ;
    int from = sizeof(sa);  // Size of the sockaddr
    char packet[512];
    make_packet(packet, buffer, size, pid); // Convert to a tagged packet
    if ((ibytessent = sendto(sock,packet,packet_size,0,(SOCKADDR*)&sa, from)) == SOCKET_ERROR){
        throw "Send failed"; 
    }else{
       memset(buffer,0,size);  // Zero the buffer
        return ibytessent - sizeof(int);      // Return the number of sent bytes
    }   
}

/**
 * Receieve a packet over a UDP socket
 * Accepts a tagged packet over a socket and converts to packet data and tag
 */
int recv_packet(SOCKET sock, SOCKADDR_IN sa, char* buffer, int size, int pid){
    int ibytesrecv, result;
    int from = sizeof(sa);
    int packet_size = size ;
    char packet[512];
    fd_set readfds;                   // Used by select to manage file descriptor multiplexing

    struct timeval *tp=new timeval;   // Timeout struct
    tp->tv_sec=0;                     // Set current time
    tp->tv_usec=TIMEOUT_USEC;         // Set timeout time

    FD_ZERO(&readfds);
    FD_SET(sock,&readfds);
    if((result=select(1,&readfds,NULL,NULL,tp))==SOCKET_ERROR) throw "Timer error!";
    else if(result > 0){
        memset(packet,0,packet_size);
        if((ibytesrecv = recvfrom(sock, packet, packet_size,0,(SOCKADDR*)&sa, &from)) == SOCKET_ERROR){
            throw "Recv failed";
        }else{
            int packet_id;
          memset(buffer,0,size); // Clear the buffer to prepare to receive data
            split_packet(packet, buffer, size, &packet_id);
            return packet_id;
        }
    }else{
        return -1;
    }
}


// Performs a safe send, loops sending and then sends an ack
int send_safe(SOCKET sock, SOCKADDR_IN sa, char* buffer, int size, int pid){
    char ackbuf[128];
    memset(ackbuf, 0, sizeof(ackbuf));
    send_packet(sock, sa, buffer, size, pid);
    return recv_packet(sock, sa, ackbuf, size, pid);
}


// Performs a safe recv function, only sends the WOOT ack on success
int recv_safe(SOCKET sock, SOCKADDR_IN sa, char* buffer, int size, int pid){
    char ackbuf[128];
    memset(ackbuf, 0, sizeof(ackbuf));
    strncpy(ackbuf, "WOOT", 4);
    int result;
    if((result = recv_packet(sock, sa, buffer, size, pid)) == pid)
        send_packet(sock, sa, ackbuf, size, pid);
    return result;
}

/**
 * GET function
 * Performs the receiving half of a request
 */
void get(SOCKET s, SOCKADDR_IN sa, char * username, char* filename, int client_num, int server_num, FILE* logfile){
    char buffer[FRAME_SIZE];
    int count, offset, recv, filesize, size;
    char tracebuf[128];

    FILE* recv_file = fopen(filename, "wb");

    if(recv_safe(s, sa, buffer, FRAME_SIZE, 101) == 101){ // Receives the filesize negotiation packet

        memcpy(&filesize, buffer + (3 * sizeof(char)), sizeof(int));

        cout << "Got filesize " << filesize << " starting transfer..." << endl;

        sprintf(tracebuf, "Filesize %d", filesize);
        write_log(logfile, username, tracebuf);

        offset = recv = count = 0;

        int expected_size = WINDOW_SIZE + 1;
        int recv_count, nak;
        int next = 0;
        int packet_id;
        // Receive the file
        while(1){
            nak = -1;
            recv_count = 0;
            next = offset;
            while(count < filesize && recv_count < WINDOW_SIZE){
                if(filesize - count >= (FRAME_SIZE))    size = (FRAME_SIZE / sizeof(char));         // Read the full buffer
                else                                    size = ((filesize - count) / sizeof(char)); // Read a subset of the buffer
                if((packet_id = recv_packet(s,sa,buffer,FRAME_SIZE,offset)) == offset){ // Receive the packet from the peer
                    count += FRAME_SIZE;
                    fwrite(buffer,sizeof(char),size,recv_file);     // Write to the output file
                    
                    sprintf(tracebuf, "Recv %d (%d of %d)", offset, count, filesize);
                    write_log(logfile, username, tracebuf);

                    offset = (offset + 1) % expected_size;    
                    recv_count++;
                }else if(packet_id < 0){
                    nak = offset;
                    break;
                }else if(packet_id == 101){
                    fclose(recv_file);
                    return get(s, sa, username, filename, client_num, server_num, logfile);
                }
            }
            while(recv_count > 0 || nak >= 0){
                memset(buffer,0,FRAME_SIZE);
                if(next != nak) strncpy(buffer, "ACK", 3);  // Send ACK
                else            strncpy(buffer, "NAK", 3);  // Send NAK
                send_packet(s,sa,buffer,FRAME_SIZE,next); // Send acknowledgement
                recv_count--;
                if(next == nak){
                    offset = nak;
                    sprintf(tracebuf, "Sent NAK for %d", nak);
                    write_log(logfile, username, tracebuf);
                    break; 
                } // As soon as we send a NAK we can break
                next = (next + 1) % expected_size;
            }

            if(count >= filesize) break;
        }
        strncpy(buffer, "ALL", 3);
        send_packet(s, sa, buffer, FRAME_SIZE, next);
        cout << "Transfer completed! " << count << " bytes received" << endl;
        fclose(recv_file);
    }else{
        fclose(recv_file);
        return get(s, sa, username, filename, client_num, server_num, logfile);
    }
}

/**
 * PUT function
 * Performs the sending half of a request
 */
void put(SOCKET s, SOCKADDR_IN sa, char * username, char* filename, int client_num, int server_num, FILE* logfile){

    char window[FRAME_SIZE * WINDOW_SIZE];  // data retention window
    char buffer[FRAME_SIZE];                // send buffer
    int filesize;
    int size = 0, sent = 0;                 // Trace variables
    char tracebuf[128];

    FILE* send_file;

    if((send_file = fopen(filename, "rb")) != NULL){    // open the file

        // Determines the file size
        fseek(send_file, 0L, SEEK_END);
        filesize = ftell(send_file);
        fseek(send_file, 0L, SEEK_SET);

        sprintf(tracebuf, "Filesize %d", filesize);
        write_log(logfile, username, tracebuf);

        strncpy(buffer, "SIZ", 3);
        memcpy(buffer + (3 * sizeof(char)), &filesize, sizeof(int)); // Add the size of the element to the buffer
        if(send_safe(s,sa,buffer,FRAME_SIZE,101) == 101){

            cout << "Sent filesize, starting transfer..." << endl;

            memset(buffer, 0, sizeof(buffer));

            int count = 0;
            int offset = 0;
            int frames_outstanding = 0;
            int next = 0;
            bool resend = false;
            int packet_id;
            int pid_max = WINDOW_SIZE + 1;

            // Start sending the file
            while (1){
                // If the acks mismatch with the current send offset, has to be a resend
                if(next != offset && frames_outstanding > 0) resend = true;

                // Send as many frames as available for the given window size
                while((!feof(send_file) && frames_outstanding < WINDOW_SIZE) || resend){
                    if(next == offset) resend = false;

                    if(!resend){
                        if(feof(send_file)) break;
                        fread(buffer,1,FRAME_SIZE,send_file);                       // Read the next block of data
                        memcpy(window + (offset * FRAME_SIZE), buffer, FRAME_SIZE); // Store the data in the local window
                        send_packet(s,sa,buffer,FRAME_SIZE,offset);             // Send the packet to peer
                        offset = (offset + 1) % pid_max;                        // Update the offset
                        frames_outstanding++;
                    }else{
                        // Resend by copying the data from the window
                        memcpy(buffer, window + (next * FRAME_SIZE), FRAME_SIZE);
                        send_packet(s,sa,buffer,FRAME_SIZE,next);
                        sprintf(tracebuf, "Resending packet %d", next);
                        write_log(logfile, username, tracebuf);
                        next = (next + 1) % pid_max;
                    }
                }

                // Receive ACKs before continuing sending 
                while(frames_outstanding > 0){
                    if((packet_id = recv_packet(s,sa,buffer,FRAME_SIZE,next)) < 0){
                        if(count < filesize) resend = true;
                        //else frames_outstanding --;
                        break;
                    }
                    // Receive acknowledgment from the client
                    if(!strncmp(buffer,"NAK", 3)){
                        if(packet_id >= 0) next = packet_id;    // Set the next packet id to send
                        break;
                    }else if(!strncmp(buffer,"ALL", 3)){
                        frames_outstanding = 0;
                        break;
                    }
                    count += FRAME_SIZE;                    // Increment the counter
                    sprintf(tracebuf, "Sent %d bytes", count);
                    write_log(logfile, username, tracebuf);
                    memset(buffer, 0, sizeof(buffer));      // Zero the buffer
                    next = (next + 1) % pid_max;            // Update the next frame tracker
                    frames_outstanding --;                  // Another frame has been acked
                }

                if(feof(send_file) && frames_outstanding == 0) break; // Break when done reading the file and all frames are acked
            }
            cout << "File transfer completed" << endl;
            fclose(send_file);
        }else{
            fclose(send_file);
            return put(s,sa,username,filename, client_num, server_num, logfile);
        }
    }

}





