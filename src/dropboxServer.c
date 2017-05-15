#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MIN_ARG 1
#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 256


void run_thread(void *socket_client)
{
	char buffer[BUFFER_SIZE];
	int socket = *(int*)socket_client;
	int message;
	
	bzero(buffer, BUFFER_SIZE);
	
	//read
	message = read(socket, buffer, BUFFER_SIZE);
	if (message < 0) 
		printf("ERROR reading from socket");
	printf("Here is the message: %s\n", buffer);
	
	// write
	message = write(socket,"I got your message", 18);
	if (message < 0) 
		printf("ERROR writing to socket");

	close(socket);
}


void socket_thread()
{



	// TODO thread fuction


	printf("Foi");
}

int main(int argc, char *argv[])
{
	int socket_connection, socket_client;
	socklen_t client_len;
	struct sockaddr_in serv_addr, client_addr;

	
	if(argc <= MIN_ARG)
	{
		printf("Not enough arguments passed.");
		exit(1);
	}
	
	int PORT = atoi(argv[1]);
	
	if ((socket_connection = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (bind(socket_connection, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");
	
	listen(socket_connection, MAX_CONNECTIONS);
	client_len = sizeof(struct sockaddr_in);
	
	while( (socket_client = accept(socket_connection, (struct sockaddr *) &client_addr, &client_len)) )
	{
		pthread_t client_thread;
		
		pthread_create(&client_thread, NULL, run_thread, (void*)socket_thread);
		
		pthread_detach(client_thread);
		
	}
	
	return 0; 
}
