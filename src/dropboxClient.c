#include "../include/dropboxClient.h"
#include "../include/dropboxUtil.h"

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
	// se o sync_dir nao existir, cria-lo
	if (stat("~/sync_dir", &st) == -1) {
		mkdir("~/sync_dir", 0700);
	}

	// cliente deve ver se existem arquivos do servidor para espelhar na pasta sync_dir
	
	//

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

	//toda vez que criar um cliente ele vai ter uma estrutura de client com seu username para passar pro servidor armazenar na lista de gente conectados
	struct client self;
	//self.userid = argv[1];
	//self.devices = {0,0};
	//função que verifica se existe a pasta sync_dir na home do cliente	
	self.logged_in = 0;

	// conecta este cliente com o servidor, que criará uma thread para administrá-lo
	socketfd = connect_server(argv[2], atoi(argv[3]));
    
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
