#include "../include/dropboxClient.h"
#include "../include/dropboxUtil.h"

struct client self;

/*
	TODO:
		- NA MAIN
			- criar o loop de pegar o input do usuário e executar o comando
			- ver como fica o comando list

		DEPOIS QUE ARRUMAR ISSO TUDO:
		- ver os mutex!

*/

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

//VIROU A FUNÇÃO DA THREAD SEPARADA DO DAEMON
void sync_client(*(int*)socket_sync)
{
	
	char buffer[BUFFER_SIZE];
	int socketfd = *(int*)socket_sync;

	while(1)
	{
		// faz isso a cada x segundos:
		sleep(SLEEP);
		
		struct client server_mirror;
		struct file_info *fi;
	
		char[256] home = "/home/";
		strcat(home, getlogin());
		update_client(&self, home);
	
		// envia para o servidor que ele vai começar o sync.
		bzero(buffer, BUFFER_SIZE);
		memcpy(buffer, SYNC, 1);
		write(socketfd, buffer, BUFFER_SIZE);

		// envia para o servidor o seu login
		bzero(buffer,BUFFER_SIZE);
		memcpy(buffer, self.userid, MAXNAME);
		write(socketfd, buffer, BUFFER_SIZE);

	  	// fica esperando o servidor enviar sua estrutura deste client.
		bzero(buffer,BUFFER_SIZE);
		read(socketfd, buffer, BUFFER_SIZE);
		memcpy(&server_mirror, buffer, sizeof(struct client));
	  
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
						bzero(buffer, BUFFER_SIZE);
						memcpy(buffer, DOWNLOAD, 1);
						write(socketfd, buffer, BUFFER_SIZE);

						bzero(buffer,BUFFER_SIZE);
						memcpy(buffer, server_mirror.fileinfo[i].name, MAXNAME);
						write(socketfd, buffer, BUFFER_SIZE);
					
						struct file_info f;
						//fica esperando receber struct
						bzero(buffer,BUFFER_SIZE);
						read(socketfd, buffer, BUFFER_SIZE);
						memcpy(&f, buffer, sizeof(struct file_info));
				
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
						bzero(buffer, BUFFER_SIZE);
						memcpy(buffer, DOWNLOAD, 1);
						write(socketfd, buffer, BUFFER_SIZE);

						bzero(buffer,BUFFER_SIZE);
						memcpy(buffer, server_mirror.fileinfo[i].name, MAXNAME);
						write(socketfd, buffer, BUFFER_SIZE);

						struct file_info f;
						//fica esperando receber struct
						bzero(buffer,BUFFER_SIZE);
						read(socketfd, buffer, BUFFER_SIZE);
						memcpy(&f, buffer, sizeof(struct file_info));
				
						//recebe arquivo
						//TODO

						//bota f na estrutura self
						insert_file_into_client_list(&self, f);
					}
					else
					{
						// o arquivo é velho e deve ser deletado do servidor adequadamente.
						bzero(buffer, BUFFER_SIZE);
						memcpy(buffer, DELETE, 1);
						write(socketfd, buffer, BUFFER_SIZE);

						bzero(buffer,BUFFER_SIZE);
						memcpy(buffer, server_mirror.fileinfo[i].name, MAXNAME);
						write(socketfd, buffer, BUFFER_SIZE);
					}
				}
		    }
		}

		//avança o estado de commit do cliente para o mesmo do servidor, já que ele atualizou.
		if(self.current_commit < server_mirror.current_commit)
			self.current_commit = server_mirror.current_commit + 1;
		else
			self.current_commit += 1;

		// AQUI ETAPA DO SYNC_SERVER!
	}
}

int main(int argc, char *argv[])
{
    int socketfd;
    char buffer[BUFFER_SIZE];
	char message[BUFFER_SIZE];
	int sync_socketfd;

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

	// recebe um ok do servidor para continuar a conexão

	// conecta um novo socket na porta +1 para fazer o sync apenas sem bloquear o programa de comandos.
	sync_socketfd = connect_server(argv[2], atoi(argv[3])+1);

	// dispara nova thread pra fazer o sync_client
	pthread_t initial_sync_client;
	pthread_create(&initial_sync_client, NULL, sync_client, (void *)&sync_socketfd);
	pthread_detach(initial_sync_client);
    

	// REVER ISSO
	while(1) 
	{
		bzero(buffer, BUFFER_SIZE);
		fgets(buffer, BUFFER_SIZE, stdin);

		strcpy(message, buffer);

		char command[256] = strtok(message, " ");
		if(strcmp(command, "list") == 0)
		{

		}
		else if(strcmp(command, "exit") == 0)
		{
			bzero(buffer, BUFFER_SIZE);
			memcpy(buffer, 'e', 1);
			write(socketfd, buffer, BUFFER_SIZE);
		}
		else if(strcmp(command, "upload") == 0)
		{
			char filepath[256] = strtok(message, NULL);

			bzero(buffer, BUFFER_SIZE);
			memcpy(buffer, 'u', 1);
			write(socketfd, buffer, BUFFER_SIZE);

			// enviar o nome do arquivo
		}
		else if(strcmp(command, "download") == 0)
		{
			char file[256] = strtok(message, NULL);

			bzero(buffer, BUFFER_SIZE);
			memcpy(buffer, 'd', 1);
			write(socketfd, buffer, BUFFER_SIZE);

			//enviar o nome do arquivo
		}
	}

    return 0;
}
