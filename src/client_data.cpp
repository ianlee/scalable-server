#include "client_data.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: client_data.cpp - Hold the code for the client data class used by the scalable servers. 
--
-- PROGRAM: server
--
-- FUNCTIONS: ClientData* ClientData::Instance()
--			  ClientData::~ClientData()
--			  int ClientData::setFile(char* filename)
--			  int ClientData::print()
--			  int ClientData::addClient(int socket, char* client_addr, int client_port)
--			  int ClientData::removeClient(int socket)
--			  int ClientData::empty()
--			  int ClientData::has(int sock)
--			  int ClientData::setRtt(int sock)
--			  int ClientData::recordData(int socket, int number)
--			  int ClientData::getNumRequest(int socket)
--			  void ClientData::cleanup(int signum)
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- NOTES: Used by other server type classes, this class handles the list of clients in an organized fashion.
----------------------------------------------------------------------------------------------------------------------*/

//ClientData* ClientData::m_pInstance = NULL;
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: Instance
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: ClientData* ClientData::Instance()
--
-- RETURNS:  Returns the instance of class generated.
--
-- NOTES: Creates an instance of client data to be portably used by other classes.
----------------------------------------------------------------------------------------------------------------------*/
ClientData* ClientData::Instance()
{
	static ClientData m_pInstance;	

	return &m_pInstance;
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: ~ClientData
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: ClientData::~ClientData()
--
-- RETURNS:  N/A
--
-- NOTES: A class destructor that closes the file pointer, iterates through the client list 
-- 		  and close each client socket.  
----------------------------------------------------------------------------------------------------------------------*/
ClientData::~ClientData(){
	fclose(_file);
	for( std::map<int, client_data>::iterator ii=list_of_clients.begin(); ii!=list_of_clients.end(); ++ii) {
		close( (*ii).first );
	}
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: setFile
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int ClientData::setFile(const char* filename)
--				      const char* filename - file name specified
--
-- RETURNS:  0 on success
--
-- NOTES: This function opens a file under the specified filename and returns 0.
----------------------------------------------------------------------------------------------------------------------*/
int ClientData::setFile(const char* filename){
	_file = fopen(filename,"a+");
	if(_file==NULL) return -1;
	return 0;
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: print
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int ClientData::print()
--
-- RETURNS:  Number of clients in the list.
--
-- NOTES: Print number of clients and the avg RTT of clients in the specified file pointer.
----------------------------------------------------------------------------------------------------------------------*/
int ClientData::print(){
	unsigned long size =0;
	double avgRtt =0;
	double totalRtt=0;
	int numClients = 0;
	
	_mutex.lock();
	size =  list_of_clients.size() ;
	
	for(std::map<int,client_data>::iterator it = list_of_clients.begin(); it!=list_of_clients.end(); ++it){
		if(it->second.rtt !=0){
			totalRtt += it->second.rtt;
			++numClients;
		}
	}
	_mutex.unlock();
	avgRtt = totalRtt / numClients;
	fprintf(_file,"clients: %lu \tRTT: %lf \tcalcsize:%d\n", size, avgRtt,numClients);
	fflush(_file);
	return list_of_clients.size();
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: addClient
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int ClientData::addClient(int socket, char* client_addr, int client_port)
--					int socket        - client socket
--					char* client_addr - client address struct
--					int client_port   - client port
--
-- RETURNS:  0 on sucess
--
-- NOTES: Adds a client to the map container list.
----------------------------------------------------------------------------------------------------------------------*/
int ClientData::addClient(int socket, char* client_addr, int client_port){
	struct client_data tempData;
	tempData.socket=socket;
	memcpy(tempData.client_addr,client_addr, strlen( client_addr));
	tempData.client_port = client_port;
	
	tempData.num_request=0;
	tempData.rtt = 0;
	tempData.amount_data=0;
	tempData.lasttime = 0;
	tempData.last_time.tv_sec = 0;
	tempData.last_time.tv_usec = 0;
	
	
	_mutex.lock();
	list_of_clients.insert(std::pair<int, client_data>(socket, tempData));
	_mutex.unlock();
	return 0;
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: removeClient
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int ClientData::removeClient(int socket)
--
-- RETURNS:  0 on success
--
-- NOTES: Erases the client data from the list based on the client socket passed in.
----------------------------------------------------------------------------------------------------------------------*/
int ClientData::removeClient(int socket){
	_mutex.lock();
	std::map<int,client_data>::iterator data = list_of_clients.find(socket);
	_mutex.unlock();
	if(data != list_of_clients.end()){
		std::cout<<"Disconnected: socket:"<<socket<<"\thostname:"<< data->second.client_addr<<"\t#requests: "<< data->second.num_request<< "\t#data: "<< data->second.amount_data << std::endl;
	}
	_mutex.lock();
	list_of_clients.erase(socket);
	_mutex.unlock();
	return 0;
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: empty
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int ClientData::empty()
--
-- RETURNS:  true if the number of active clients are empty, false otherwise.
--
-- NOTES: Returns true or false if the number of clients in the list are empty.
----------------------------------------------------------------------------------------------------------------------*/
int ClientData::empty(){
	int empty;
	_mutex.lock();
	empty = list_of_clients.empty();
	_mutex.unlock();
	return empty;

}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: has
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int ClientData::has(int sock)
--			  	  int sock - client socket passed in
--
-- RETURNS:  true if the map contains the client data whose socket is matched to.
--
-- NOTES: Returns true or false if the map container has the client data with the matching client socket.
----------------------------------------------------------------------------------------------------------------------*/
int ClientData::has(int sock){
	_mutex.lock();
	int rtn = list_of_clients.count(sock);
	_mutex.unlock();
	return rtn;
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: setRtt
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int ClientData::setRtt(int socket)
--                            int socket - socket for which rtt should be calculated for.
--
-- RETURNS:  calculated ReturnTripTime in milliseconds.  if it has no previous time value, returns -1
--
-- NOTES: calculates the RTT if previous timeval last_time exists.  Sets last_time to current time
----------------------------------------------------------------------------------------------------------------------*/
int ClientData::setRtt(int socket){
	int rtt = -1;
	struct timeval currTime;
	gettimeofday(&currTime,NULL);
	long thistime = currTime.tv_sec * 1000000 + currTime.tv_usec;

	_mutex.lock();
	std::map<int,client_data>::iterator data = list_of_clients.find(socket);
	_mutex.unlock();
	if(data != list_of_clients.end()){
	
		if(data->second.lasttime > 0){
			
			rtt = thistime- data->second.lasttime;
			data->second.rtt = rtt;
		}
		data->second.lasttime = thistime;
		++ data->second.num_request;
	}
	
	return rtt;
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: recordData
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int ClientData::recordData(int socket, int number)
--                            int socket - socket for which data should be recorded for.
--                            int number - number of bytes sent to this client
--
-- RETURNS:  total amount of data sent to this client.
--
-- NOTES: saves amount of data sent by socket.
----------------------------------------------------------------------------------------------------------------------*/
int ClientData::recordData(int socket, int number){
	long total = 0;
	_mutex.lock();
	std::map<int,client_data>::iterator data = list_of_clients.find(socket);
	_mutex.unlock();
	if(data != list_of_clients.end()){
		data->second.amount_data += number;
		total = data->second.amount_data;
	}
	return total;
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: getNumRequest
--
-- DATE: 2014/02/21
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ian Lee, Luke Tao
--
-- PROGRAMMER: Ian Lee, Luke Tao
--
-- INTERFACE: int ClientData::getNumRequest(int socket)
--                            int socket - socket for which number of requests should be returned
--
-- RETURNS:  number of times received data
--
-- NOTES: getter function for number of requests received
----------------------------------------------------------------------------------------------------------------------*/
int ClientData::getNumRequest(int socket){
	_mutex.lock();
	std::map<int,client_data>::iterator data = list_of_clients.find(socket);
	_mutex.unlock();
	if(data != list_of_clients.end()){
		return data->second.num_request;
	}
	return 0;
}


void ClientData::cleanup(int signum){

	exit(signum);
}
