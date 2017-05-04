#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MIN_AGR 1
#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 256

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, n;
	socklen_t client_len;
	struct sockaddr_in serv_addr, client_addr;
	char buffer[BUFFER_SIZE];
	
	if(argc < MIN_ARG)
	{
		printf("Not enough arguments passed.");
		exit(1);
	}
	
	int PORT = atoi(argv[1]);
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");
	
	while(true)
	{
		listen(sockfd, MAX_CONNECTIONS);
		
		client_len = sizeof(struct sockaddr_in);
		if ((newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len)) == -1) 
			printf("ERROR on accept");
		
		//http://stackoverflow.com/questions/3719462/sockets-and-threads-using-c
		//o problema aqui é: o newsockfd nao vai ficar sendo sobreescrito sempre que criar uma nova thread?
		//devemos fazer o accept dentro da thread talvez?
		//se não for dentro da thread, onde a gente da close no socket?
		pthread_t thread;
		pthread_create(&thread, NULL, run_client, &newsockfd);
		
		bzero(buffer, BUFFER_SIZE);
		
		/* read from the socket */
		n = read(newsockfd, buffer, BUFFER_SIZE);
		if (n < 0) 
			printf("ERROR reading from socket");
		printf("Here is the message: %s\n", buffer);
		
		/* write in the socket */ 
		n = write(newsockfd,"I got your message", 18);
		if (n < 0) 
			printf("ERROR writing to socket");

		close(newsockfd);
		close(sockfd);
	}
	
	return 0; 
}
