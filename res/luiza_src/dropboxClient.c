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
	struct file_info *fi;
	
	char[256] home = "/home/";
	strcat(home, getlogin());
	update_client(&self, home);
	
	// envia para o servidor que ele vai começar o sync.
	send(socketfd, SYNC, 1, 0);

	// envia para o servidor o seu login
	//NÃO É MELHOR ELE ENVIAR ISSO UMA VEZ NO INICIO E O SERVER GUARDAR???
	send(socketfd, self.userid, sizeof(self.userid), 0);

  	// fica esperando o servidor enviar sua estrutura deste client.
    recv(socketfd, server_mirror, sizeof(struct client), 0);
  
  	// pra cada arquivo do servidor:
  	int i;
  	for(i = 0; i < MAXFILES; i++) 
    {
      	if(strcmp(server_mirror.fileinfo[i].name, "") == 0)
           break;
    	else
        {
        	// arquivo existe no cliente?
			*fi = search_files(&self, server_mirror.fileinfo[i].name);

			if(fi != NULL)		// arquivo existe no cliente
			{
				//verifica se o arquivo no servidor tem commit_created/modified > state do cliente.
				if(server_mirror.fileinfo[i].commit_modified > self.current_commit)
				{
					//isso quer dizer que o arquivo no servidor é de um commit mais novo que o estado atual do cliente.
					// pede para o servidor mandar o arquivo
					send(socketfd, DOWNLOAD, 1, 0);
					send(server_mirror.fileinfo[i].name, MAXNAME, 0);
					
					struct file_info f;
					//fica esperando receber struct
					recv(socketfd, f, sizeof(struct file_info);
				
					//recebe arquivo
					//TODO

					// atualiza na estrutura do cliente.
					*fi = f;
				}
			}
			else				// arquivo não existe no cliente
			{
				// verifica se o arquivo no servidor tem um commit_modified > state do cliente
				if(server_mirror.fileinfo[i].commit_modified >= self.current_commit)
				{
					//isso quer dizer que é um arquivo novo colocado no servidor em outro pc.
					// pede para o servidor mandar o arquivo
					send(socketfd, DOWNLOAD, 1, 0);
					send(server_mirror.fileinfo[i].name, MAXNAME, 0);

					struct file_info f;
					//fica esperando receber struct
					recv(socketfd, f, sizeof(struct file_info);
				
					//recebe arquivo
					//TODO

					//bota f na estrutura self
					insert_file_into_client_list(&self, f);
				}
				else
				{
					// o arquivo é velho e deve ser deletado do servidor adequadamente.
					send(socketfd, DELETE, 1, 0);
					send(server_mirror.fileinfo[i].name, MAXNAME, 0);
				}
			}
        }
    }

	//avança o estado de commit do cliente para o mesmo do servidor, já que ele atualizou.
	if(self.current_commit < server_mirror.current_commit)
		self.current_commit = server_mirror.current_commit + 1;
	else
		self.current_commit += 1;
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
	
	init_client(&self, home, argv[1]);

	// conecta este cliente com o servidor, que criará uma thread para administrá-lo
	socketfd = connect_server(argv[2], atoi(argv[3]));

	//send userid to server
	send(socketfd, self.userid, MAXNAME, 0);

	// dispara nova thread pra fazer o sync_client
	// SOCORRO???? COMO QUE FICA DO LADO DO SERVIDOR???
	pthread_t initial_sync_client;
	pthread_create(&initial_sync_client, NULL, sync_client, NULL);
	pthread_detach(initial_sync_client);
	//	sync_client();
    
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
