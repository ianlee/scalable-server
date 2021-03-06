#include "echo_client.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: echo_client.cpp - Hold the code for the client class used by the scalable servers. 
--
-- PROGRAM: echo_client
--
-- FUNCTIONS: Client::Client(char * host, int port, int t_sent)
--			  int Client::run()
--			  int Client::create_socket()
--			  int Client::connect_to_server(int socket, char * host)
--			  int Client::send_msgs(int socket, char * data)
--			  int Client::recv_msgs(int socket, char * buf)
--			  int Client::setBufLen(int buflen)
--			  int Client::setConnections(int connections)
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
	int rtn;
	char sendBuf[]={ "FOOBAR "};

	char recvBuf[_buflen];
	int nready, epoll_fd;
	struct epoll_event events[_connections], event;
	//Create multiple processes and each process will be a single client essentially

	epoll_fd = epoll_create(_connections);
	if (epoll_fd == -1) 
		fprintf(stderr,"epoll_create\n");
	// Add the server socket to the epoll event loop
	for(int i = 0; i < _connections; i++){
		//create clients and add to epoll
		int clientSock = create_socket();
		if(connect_to_server(clientSock, _host)<=0){
			fprintf(stderr,"connect\n");
				exit(1);
		}
		if (fcntl (clientSock, F_SETFL, O_NONBLOCK | fcntl (clientSock, F_GETFL, 0)) == -1) {
			fprintf(stderr,"fcntl\n");
				exit(1);
		}
		
		if(clientSock >0){
			event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
			event.data.fd = clientSock;
			if (epoll_ctl (epoll_fd, EPOLL_CTL_ADD, clientSock, &event) == -1) {
				fprintf(stderr,"epoll_ctl\n");
				exit(1);
			}
				
			
			rtn = send_msgs(clientSock, sendBuf);
			ClientData::Instance()->recordData(clientSock, rtn);
		}
	}
	fflush(stderr);
	
	
	
	while(true){
		
		nready = epoll_wait (epoll_fd, events, _connections, -1);
		for (int i = 0; i < nready; i++){	// check all clients for data
			int sock = events[i].data.fd;
			// Case 1: Error condition
    		if (events[i].events & (EPOLLHUP | EPOLLERR)) {
				fputs("epoll: EPOLLERR", stderr);

				close(sock);
				ClientData::Instance()->removeClient(sock);
				continue;
    		}
    		assert (events[i].events & EPOLLIN);
    		// Case 2: One of the sockets has read data
			int rtn = recv_msgs(sock, recvBuf);
			if(rtn){
				
				//do rtt calc
				ClientData::Instance()->setRtt(sock);
				
			}
			else{
				continue;
			}
			//do # client sent calc
			if(ClientData::Instance()->getNumRequest(sock) < times_sent){
				rtn = send_msgs(sock, sendBuf);
				ClientData::Instance()->recordData(sock, rtn);
			} else {
				//met quota for sending packets to server. close connection
				ClientData::Instance()->removeClient(sock);
				close(sock);
				
			}
 		}
		if(ClientData::Instance()->empty()){
			break;
		}
	
	}
	

	std::cout << "All clients finished processing" << std::endl;
	return 0;
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
		exit(1);
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
		fflush(stderr);
		perror("connect");
		exit(1);
		return 0;
	}
	if(socket > 0){
		ClientData::Instance()->addClient(socket, inet_ntoa(server.sin_addr),server.sin_port );
	}

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
	return send(socket, data, _buflen, 0);
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
	
	int n, bytes_to_read = _buflen;
	while ((n = recv (socket, buf, bytes_to_read, 0)) < bytes_to_read)
	{
		
		if(n == -1){
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			}
			printf("error %d %d %d\n", bytes_to_read, n, socket);
			printf("error %d\n",errno);
			ClientData::Instance()->removeClient(socket);
			close(socket);
				exit(1);
			return -1;
		} else if (n == 0){
			printf("socket was gracefully closed by other side %d\n",socket);
			ClientData::Instance()->removeClient(socket);
			close(socket);
				exit(1);
			return -1;
		}
		buf += n;
		bytes_to_read -= n;
	}

	return 1;
}

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: setBufLen
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int Client::setBufLen(int buflen)
--				    int buflen - length of buffer 
--				
--
-- RETURNS:  1
--
-- NOTES: sets buffer length.
----------------------------------------------------------------------------------------------------------------------*/
int Client::setBufLen(int buflen){
	_buflen=buflen;
	return 1;

}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: setBufLen
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int Client::setConnections(int connections)
--				    int connections - max number of connections
--				
--
-- RETURNS:  1
--
-- NOTES: sets max connections.
----------------------------------------------------------------------------------------------------------------------*/
int Client::setConnections(int connections){
	_connections = connections;
	return 1;
}
