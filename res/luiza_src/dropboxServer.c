#include "../include/dropboxServer.h"
#include "../include/dropboxUtil.h"


void receive_file(char *file, int client_socket)
{
	int fd, verification_fd;
	struct stat file_stat;
	ssize_t len;
	int remain_data;
	char file_size[256];
	int offset;
	int sent_bytes = 0;

	// Open file
	verification_fd = open(file, O_RDONLY);
	if (verification_fd == -1)
    {
		// File doesn't exist yet, it must be created
		fd = open(file, O_CREAT, S_IRUSR); // | S_IWUSR | S_IXUSR);
    }
	else
	{
		// File already exist
		fd = open(file, O_APPEND);
	}
	if (fd == -1)
    {
		printf("Error opening file");
		exit(EXIT_FAILURE);
    }

	// Receive file data
	// TODO
}

void send_file(char *file, int client_socket)
{
	int fd, verification_fd;
	struct stat file_stat;
	ssize_t len;
	int remain_data;
	char file_size[256];
	int offset;
	int sent_bytes = 0;

	// Open file
	fd = open(file, O_RDONLY);
	if (fd == -1)
    {
		printf("Error opening file");
		exit(EXIT_FAILURE);
    }

	// Get file stats
	if (fstat(fd, &file_stat) < 0)
	{
		printf("Error fstat");
		exit(EXIT_FAILURE);
	}

	// Send file size
	remain_data = file_stat.st_size;
	itoa(remain_data, file_size, 10);
	len = send(client_socket, file_size, sizeof(file_size), 0);
	if (len < 0)
	{
		printf("Error on sending file size");
		exit(EXIT_FAILURE);
	}
	printf("Server sent %d bytes for the file size\n", len);

	// Send file data
	offset = 0;
	while (((sent_bytes = sendfile(client_socket, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0))
	{
		remain_data -= sent_bytes;
		printf("Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
	}
}

void run_thread(void *socket_client)
{
	char buffer[BUFFER_SIZE];
	int socketfd = *(int*)socket_client;
	char message;
	printf("i created a thread\n");
	
	while(1) {
		bzero(buffer, BUFFER_SIZE);
		message = read(socketfd, buffer, BUFFER_SIZE);
		if (message < 0) 
			printf("ERROR reading from socket");
		else {
			memcpy(&message, buffer, 1);

			//nao sei se é exatamente assim
			switch(message){
				case EXIT:
					disconnect_client(client);
					pthread_exit();
					break;
				case SYNC:			// SERVER RECEBE SYNC -> CLIENT ESTÁ FAZENDO SYNC. 
					{
						char client_id[MAXNAME];
						//recebe id do cliente. ---> VER SE NÃO É MELHOR ELE RECEBER ANTES???
						//pegar os dados do buffer
						read(socketfd, buffer, BUFFER_SIZE);
						memcpy(client_id, buffer, MAXNAME);
						
						bzero(buffer,BUFFER_SIZE);

						// pega cliente na lista de clientes e envia o mirror para o cliente.

						memcpy(buffer, &client, sizeof(client));
						write(socketfd, buffer, BUFFER_SIZE);

						// agora fica em um while !finished, fica recebendo comandos de download/delete
						while(1)
						{	
							bzero(buffer,BUFFER_SIZE);

							char command;
							char fname[MAXNAME];

							read(socketfd, buffer, BUFFER_SIZE);
							memcpy(&command, buffer, 1);

							if(command == DOWNLOAD)
							{
								bzero(buffer,BUFFER_SIZE);

								// recebe nome do arquivo
								read(socketfd, buffer, BUFFER_SIZE);
								memcpy(fname, buffer, MAXNAME);
								
								// procura arquivo
								file_info *f = search_files(client, fname);

								bzero(buffer,BUFFER_SIZE);

								// manda struct
								memcpy(buffer, &f, sizeof(file_info));
								write(socketfd, buffer, BUFFER_SIZE);
								
								// manda arquivo
							}
							else if(command == DELETE)
							{
								bzero(buffer,BUFFER_SIZE);

								// recebe nome do arquivo
								read(socketfd, buffer, BUFFER_SIZE);
								memcpy(fname, buffer, MAXNAME);

								// procura arquivo

								// deleta arquivo da pasta sync

								// deleta estrutura da lista
							}
							else
								break;
						}
						
						// aí executa aqui o sync_server.
						sync_server(socketfd);	
					}
					break;
				case DOWNLOAD:
					//tem que ver como vamos receber isso...
					message = read(socketfd, buffer, BUFFER_SIZE);
					send_file(message, socketfd);
					break;
				case UPLOAD:
					message = read(socketfd, buffer, BUFFER_SIZE);
					receive_file(message, socketfd);
					break;
			}
		}
		
	}

	free(socket_client);

	close(socketfd);
}

void sync_server(int socketfd)
{
	struct client client_mirror;
	struct file_info *fi;

	char buffer[BUFFER_SIZE];
	
	//server fica esperando o cliente enviar seu mirror
	read(socketfd, buffer, BUFFER_SIZE);
	memcpy(&client_mirror, buffer, sizeof(struct client));
	
	bzero(buffer,BUFFER_SIZE);

	char[256] home = "/home/";
	strcat(home, getlogin());
	strcat(home, "/server");

	// TODO: função que recupera o cliente com client_mirror->userid da lista de clientes.
	client *client;
	update_client(client, home);
  
  	// pra cada arquivo do cliente:
  	int i;
  	for(i = 0; i < MAXFILES; i++) 
    {
      	if(strcmp(client_mirror.fileinfo[i].name, "") == 0)
           break;
    	else
        {
        	// arquivo existe no servidor?
			*fi = search_files(client, client_mirror.fileinfo[i].name);

			if(fi != NULL)		// arquivo existe no servidor
			{
				//verifica se o arquivo no cliente tem commit_created/modified > state do servidor.
				if(client_mirror.fileinfo[i].commit_modified > client->current_commit)
				{
					//isso quer dizer que o arquivo no servidor é de um commit mais novo que o estado atual do cliente.
					// pede para o cliente mandar o arquivo
					memcpy(buffer, DOWNLOAD, 1);
					write(socketfd, buffer, BUFFER_SIZE);

					bzero(buffer,BUFFER_SIZE);

					memcpy(buffer, &client_mirror.fileinfo[i].name, MAXNAME);
					write(socketfd, buffer, BUFFER_SIZE);
		
					bzero(buffer,BUFFER_SIZE);

					struct file_info f;
					//fica esperando receber struct
					read(socketfd, buffer, BUFFER_SIZE);
					memcpy(&f, buffer, sizeof(struct file_info));
				
					//recebe arquivo
					//TODO

					// atualiza na estrutura do cliente no servidor.
					*fi = f;
				}
			}
			else				// arquivo não existe no servidor
			{
				// verifica se o arquivo no cliente tem um commit_modified > state do servidor
				if(client_mirror.fileinfo[i].commit_modified > client->current_commit)
				{
					//isso quer dizer que é um arquivo novo colocado no servidor em outro pc.
					// pede para o cliente mandar o arquivo
					memcpy(buffer, DOWNLOAD, 1);
					write(socketfd, buffer, BUFFER_SIZE);

					bzero(buffer,BUFFER_SIZE);

					memcpy(buffer, &client_mirror.fileinfo[i].name, MAXNAME);
					write(socketfd, buffer, BUFFER_SIZE);

					struct file_info f;
					//fica esperando receber struct
					recv(socketfd, f, sizeof(struct file_info);
				
					//recebe arquivo
					//TODO

					//bota f na estrutura self
					//TODO
					insert_file_into_client_list(client, f);
				}
			}
        }
    }

	//avança o estado de commit do cliente.
	client->current_commit += 1;
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
	
	while(1)
	{
		if( (socket_client = accept(socket_connection, (struct sockaddr *) &client_addr, &client_len)) )

		{
			printf("accepted a client\n");
			
			int *new_sock;
			new_sock = malloc(1);
        	*new_sock = socket_client;
			pthread_t client_thread;
		
			pthread_create(&client_thread, NULL, run_thread, (void*)new_sock);
		
			pthread_detach(client_thread);

			//free(new_sock);
		}
		
	}

	printf("saí do meu while (server)\n");
	
	return 0; 
}
