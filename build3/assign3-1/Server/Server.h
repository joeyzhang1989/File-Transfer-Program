#ifndef SERVER_H
#define  SERVER_H

#include <winsock.h>
#include <iostream>
#include <time.h>
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <windows.h>

#include <string.h>
#include <iostream>

#include <time.h>

#include <Math.h>


#define TIMEOUT_USEC 300000
#pragma comment(lib,"wsock32.lib") //for linker

#define RAWBUF_SIZE 512
#define ROUTER_PORT1 7000   // router port number 1 (server)
#define ROUTER_PORT2 7001   // router port number 2 (client)
#define PEER_PORT1  5000    // peer port number 1 (server)
#define PEER_PORT2  5001    // peer port number 2 (client)
#define FRAME_SIZE  128     // Size (in bytes) of each packet
#define WINDOW_SIZE 19

#define GET "get"           // Method name for GET requests
#define PUT "put"           // Method name for PUT requests
#define HEADER "%s\t%s\t%s" // Format string for headers

    SOCKET sock;      // Define the socket to return
    SOCKADDR_IN sa;     // Define the socket address information
    HOSTENT *hp;        // Host entity details
    char hostname[21]; // Store the value of localhost
  //socket data types
    SOCKET client_socket;   // Client socket
    SOCKADDR_IN sa_out;      // fill with server info, IP, port

    char buffer[RAWBUF_SIZE]; // Buffer

   
    char router[21];                                    // Host data
    char cusername[128], filename[128], direction[3];   // Other header data
    DWORD dwusername = sizeof(cusername);               // Retains the size of the username
    char trace_data[128];

void put(SOCKET s, SOCKADDR_IN sa, char * username, char* filename, int client_num, int server_num, FILE* logfile);
void get(SOCKET s, SOCKADDR_IN sa, char * username, char* filename, int client_num, int server_num, FILE* logfile);

// Performs a safe send, loops sending and then sends an ack
int send_safe(SOCKET sock, SOCKADDR_IN sa, char* buffer, int size, int pid);

// Performs a safe recv function, only sends the WOOT ack on success
int recv_safe(SOCKET sock, SOCKADDR_IN sa, char* buffer, int size, int pid);

void prompt(const char* message, char*buffer);
SOCKET open_port(int port);
SOCKADDR_IN prepare_peer_connection(char* hostname, int port);

void write_log(FILE* logfile, char* username, char* message);
/**
 * Create a packet with an identifier tag
 * Takes a buffer of size packet_size and generates a packet of size packet_size + sizeof(int)
 */
void make_packet(char* packet, char* buffer, int buffer_size, int number);

/**
 * Split a packet into packet name and identifier
 * Takes a packet of size packet_size + sizeof(int) and extracts a packet (size packet_size) and its integer identifier
 */
void split_packet(char* packet, char* buffer, int buffer_size, int* number);

/**
 * Send Packet over a UDP socket
 * Generates a tagged packet and sends it over the supplied socket
 */
int send_packet(SOCKET sock, SOCKADDR_IN sa, char* buffer, int size, int pid);

/**
 * Receieve a packet over a UDP socket
 * Accepts a tagged packet over a socket and converts to packet data and tag
 */
int recv_packet(SOCKET sock, SOCKADDR_IN sa, char* buffer, int size, int pid);


#endif SERVER_H