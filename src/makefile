CC = g++
CFLAGS = -Wall -g -std=c++11
LDFLAGS = -lpthread

all: myprogram client
client: main_client
echo_client.o : echo_client.cpp echo_client.h client_data.h
	${CC} ${CFLAGS} -c echo_client.cpp
main_client: echo_client.o main_client.cpp client_data.o
	${CC} ${CFLAGS} main_client.cpp echo_client.o client_data.o ${LDFLAGS} -o ../client

client_data.o : client_data.cpp client_data.h
	${CC} ${CFLAGS} -c client_data.cpp

multi_thread_server.o : multi_thread_server.cpp multi_thread_server.h client_data.h
	${CC} ${CFLAGS} -c multi_thread_server.cpp ${LDFLAGS}

main_server.o : main_server.cpp multi_thread_server.h select_server.h blocking_queue.h
	${CC} ${CFLAGS} -c main_server.cpp 

select_server.o : select_server.cpp select_server.h blocking_queue.h  client_data.h
	${CC} ${CFLAGS} -c select_server.cpp
	
epoll_server.o : epoll_server.cpp epoll_server.h blocking_queue.h  client_data.h
	${CC} ${CFLAGS} -c epoll_server.cpp

myprogram : main_server.o multi_thread_server.o client_data.o select_server.o epoll_server.o
	${CC} ${CFLAGS} main_server.o multi_thread_server.o client_data.o select_server.o epoll_server.o ${LDFLAGS} -o ../server

clean:
	rm -rf *.o  *.cpp~ *.h~ ../client ../server
