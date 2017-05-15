#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

//CODIGO DE TUTORIAL DADO NA AULA 09

#define MIN_ARG 3
#define BUFFER_SIZE 256

int main(int argc, char *argv[])
{
    int socketfd, message;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
	
    if (argc <= MIN_ARG) 
	{
		fprintf(stderr,"usage %s hostname\n", argv[0]);
		exit(0);
    }
	
	int PORT = atoi(argv[3]);
	
	if ( (server = gethostbyname(argv[1])) == NULL ) 
	{
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket\n");
    
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     
	
    
	if (connect(socket,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        printf("ERROR connecting\n");


    
	while(1) //mudar tudo aqui pro trabalho
	{
		printf("Enter the message: ");
		bzero(buffer, BUFFER_SIZE);
		fgets(buffer, BUFFER_SIZE, stdin);
		
		message = write(socket, buffer, strlen(buffer));
		if (message < 0) 
			printf("ERROR writing to socket\n");

		bzero(buffer,BUFFER_SIZE);
		
		/* read from the socket */
		message = read(socket, buffer, BUFFER_SIZE);
		if (message < 0) 
			printf("ERROR reading from socket\n");

		printf("%s\n",buffer);
		
		close(socket);
	}
    return 0;
}
