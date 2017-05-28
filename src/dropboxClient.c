#include "../include/dropboxClient.h"
#include "../include/dropboxUtil.h"

struct client self;

int connect_server(char *host, int port)
{
	int socketfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

	if ( (server = gethostbyname(host)) == NULL ) 
	{
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket\n");
    
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(port);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     
	
	if (connect(socketfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        printf("ERROR connecting\n");

	return socketfd;
}

void sync_client()
{
	struct client server_mirror;
	
  	// fica esperando o servidor enviar o espelho de todos os arquivos que estao la
    recv(socketfd, server_mirror, sizeof(struct client), 0);
  
  	// recebeu. agora vai comparar com os arquivos dele
  	int i;
  	for(i = 0; i < MAXFILES; i++) 
    {
      	if(strcmp(server_mirror.fileinfo[i], "") == 0)
           break;
    	else
        {
          //jeito inteligente de procurar no array de arquivos do self pra ver se esse arquivo existe
          //se não existir, faz requisição de envio pro server
          
        }
    }
}

int main(int argc, char *argv[])
{
    int socketfd, message;
    char buffer[BUFFER_SIZE];

	if (argc <= MIN_ARG) 
	{
		fprintf(stderr,"usage %s hostname\n", argv[0]);
		exit(0);
    }

	char[256] home = "/home/";
	strcat(home, getlogin());
	
	//inicializa estrutura self do cliente
	self = map_sync_dir(char *home, char *login);

	// conecta este cliente com o servidor, que criará uma thread para administrá-lo
	socketfd = connect_server(argv[2], atoi(argv[3]));

	//send to server
	send(socketfd, self, sizeof(struct client), 0);
	sync_client();
    
	while(1) //mudar tudo aqui pro trabalho
	{
		printf("Enter the message: ");
		bzero(buffer, BUFFER_SIZE);
		fgets(buffer, BUFFER_SIZE, stdin);
		
		message = write(socketfd, buffer, strlen(buffer));
		if (message < 0) 
			printf("ERROR writing to socket\n");

		bzero(buffer,BUFFER_SIZE);
		
		/* read from the socket */
		message = read(socketfd, buffer, BUFFER_SIZE);
		if (message < 0) 
			printf("ERROR reading from socket\n");

		printf("%s\n",buffer);
		
		//close(socketfd);
	}
    return 0;
}
