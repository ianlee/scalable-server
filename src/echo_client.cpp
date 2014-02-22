#include "echo_client.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: echo_client.cpp - Hold the code for the client class used by the scalable servers. 
--
-- PROGRAM: echo_client
--
-- FUNCTIONS: Client::Client(char * host, int port, int t_sent)
--			  int Client::run()
--			  void Client::child_client_process(int client_num, int times_sent)
--			  void Client::wait_for_client_processes()
--			  int Client::create_socket()
--			  int Client::connect_to_server(int socket, char * host)
--			  int Client::send_msgs(int socket, char * data)
--			  int Client::recv_msgs(int socket, char * buf)
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- NOTES: This class serves as an echo client that will test the different types of servers.
----------------------------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: Client (constructor)
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: Client::Client(char * host, int port, int t_sent)
--			     char * host - host of the server the client is trying to connect to
--			     int port - port of the server the client is trying to connect to
--			     int t_sent - number specified by the user how many times the client will send messages
--
-- RETURNS:  N/A
--
-- NOTES: Client constructor that will initialize the server host, port, and user-defined times the packets will be
-- sent to the server.
----------------------------------------------------------------------------------------------------------------------*/
Client::Client(char * host, int port, int t_sent) : _host(host), _port(port), times_sent(t_sent) {}

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: run
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int Client::run()
--
-- RETURNS:  0 on success
--
-- NOTES: Main echo client function that will create processes for multiple clients.
----------------------------------------------------------------------------------------------------------------------*/
int Client::run()
{	
	//Create multiple processes and each process will be a single client essentially

	for(int i = 0; i < MAX_CONNECT; i++)
	{
		switch(fork())
		{
			case -1:
				std::cerr << "An error has occurred" << std::endl;
				break;
			case 0:
				child_client_process(i, times_sent);
				return 0;
			default:
				break;
		}
	}
	wait_for_client_processes();

	std::cout << "All clients finished processing" << std::endl;
	return 0;
}

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: child_client_process
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: void Client::child_client_process(int client_num, int times_sent)
--						int client_num - client process ID number
--						int times_sent - number of times the client will send 
--																 messages to the server
--
--
-- RETURNS:  void
--
-- NOTES: A client process that will send and receive messages from the server for a specified amount of time.
----------------------------------------------------------------------------------------------------------------------*/
void Client::child_client_process(int client_num, int times_sent)
{
	std::cout << "Processing client " << client_num << std::endl;
	int clientSock = create_socket();
	char sendBuf[] = {"FOOBAR"}, recvBuf[BUFLEN];
	clientSock = connect_to_server(clientSock, _host);
	if(clientSock >0){
//send_msgs(clientSock, sendBuf);
	for(int i = 0; i<50; ++i){
		std::cout << "Sending " << send_msgs(clientSock, sendBuf) << " bytes " << client_num << std::endl;
		int rtn = recv_msgs(clientSock, recvBuf);
		if(rtn){
			std::cout << "Received " << rtn << " bytes " << client_num << recvBuf << std::endl;
		}
		else{
			break;
		}
	}

//recv_msgs(clientSock, recvBuf);
	std::cout << "Closing client " << client_num << " socket" << std::endl;
	close(clientSock);
}
	//fflush(stdout);
	exit(0);
}

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: wait_for_client_processes
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: void Client::wait_for_client_processes()
--
-- RETURNS:  void
--
-- NOTES: Waits for all client processes to finish before exiting.
----------------------------------------------------------------------------------------------------------------------*/
void Client::wait_for_client_processes()
{
	while (wait(NULL) > 0) {
   		if (errno == ECHILD) { /* If there are no more child processes running */
      		break;
   		}
	}
}

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: create_socket
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int Client::create_socket()
--
-- RETURNS:  Socket Descriptor
--
-- NOTES: Creates a socket and returns the socket descriptor on successful creation.
----------------------------------------------------------------------------------------------------------------------*/
int Client::create_socket()
{
	int sd;

	// Create the socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Cannot create socket");
	}
	return sd;
	
}

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: connect_to_server
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int Client::connect_to_server(int socket, char * host)
--					    int socket - client socket passed in
--					    char * host - server host IP
--
-- RETURNS:  0 if failure to connect to server, client socket on success
--
-- NOTES: Function that the client will try to connect to the server.
----------------------------------------------------------------------------------------------------------------------*/
int Client::connect_to_server(int socket, char * host)
{
	struct sockaddr_in server;
	struct hostent	*hostptr;



	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(_port);
	if ((hostptr = gethostbyname(host)) == NULL)
	{
		fprintf(stderr, "Unknown server address\n");
		exit(1);
	}
	bcopy(hostptr->h_addr, (char *)&server.sin_addr, hostptr->h_length);

	if (connect (socket, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
		return 0;
	}
//	printf("Connected:    Server Name: %s\n", hostptr->h_name);
	//hostptr->h_addr_list;
//	printf("\t\tIP Address: %s\n", inet_ntop(hostptr->h_addrtype, *pptr, str, sizeof(str)));
	return socket;
}

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: send_msgs
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int Client::send_msgs(int socket, char * data)
--				    int socket - client socket that the data is sending from
--				    char * data - data that the client is sending to the server
--
-- RETURNS:  Returns how many bytes the client sent to the server.
--
-- NOTES: This function will send messages to the server with the client socket passed in.
----------------------------------------------------------------------------------------------------------------------*/
int Client::send_msgs(int socket, char * data)
{
	return send(socket, data, BUFLEN, 0);
}

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: recv_msgs
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int Client::recv_msgs(int socket, char * buf)
--				    int socket - client socket passed in
--				    char * buf - data that the client is receiving from the server
--
-- RETURNS:  Returns the total bytes read.
--
-- NOTES: This function will receive messages from the server with the client socket passed in.
----------------------------------------------------------------------------------------------------------------------*/
int Client::recv_msgs(int socket, char * buf)
{
	int bytes_read = 0, total_bytes_read = 0;
	int bytes_to_read = BUFLEN;

	while ((bytes_read = recv (socket, buf, bytes_to_read, 0)) < bytes_to_read)
	{
		if(bytes_read == -1){
			
			printf("error %d %d\n",errno,socket);
			
			return -1;
		} else if (bytes_read == 0){
			
			return -1;
		}

		buf += bytes_read;
		bytes_to_read -= bytes_read;
		total_bytes_read += bytes_read;
	}
	total_bytes_read += bytes_read;
	return total_bytes_read;
}
