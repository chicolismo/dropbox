//#include "../include/dropboxServer.h"
//#include "../include/dropboxClient.h"

#include "dropboxServer.h"
#include "dropboxUtil.h"
//#include "../../include/fila2.h"
#include "fila2.h"

pthread_mutex_t queue;
FILA2 connected_clients;
char home[256];

/*
	TODO:
		- NO SYNC_SERVER:
			- deletar arquivo da pasta sync_dir_<user> do servidor
			- chamar função de deletar arquivo n da estrutura do cliente
		- @LARISSA: colocar tuas funções aqui dentro do dropboxServer.c
		- criar no loop o comando list
		
		DEPOIS QUE ARRUMAR ISSO TUDO:
		- ver os mutex!
*/

void list_files(fileinfo files[256]){
    int i;
    char filename[256];      
    for(i=0;i<256;i++){
        if (strcmp(files[i].name,"") == 0)
            break;
        else{
            strcpy(filename, "");
            strcat(filename, files[i].name);
            strcat(filename, files[i].extension);

            printf("%s", filename);
        }
            
    }

}


int return_client(char* user_name, client *new_client){
    client *tempinfo;

	if(FirstFila2(&connected_clients) != 0){
		do{
			tempinfo = (client*)(GetAtIteratorFila2(&connected_clients));
			if (strcmp(tempinfo->userid, user_name) == 0){ //mesmo id = já tem um logado
			        new_client = (client*)(GetAtIteratorFila2(&connected_clients));
					return ACCEPTED;	
            }			
            else{
					return NOT_VALID;
	        }
			
				
		}while(NextFila2(&connected_clients) != 0);
    }
}


void disconnect_client(client *clientinfo){
	pthread_mutex_lock(&queue);
	
	client *tempinfo;
	tempinfo = (client*)(GetAtIteratorFila2(&connected_clients));
	while(strcmp(tempinfo->userid, clientinfo->userid) != 0){
		NextFila2(&connected_clients);
		tempinfo = (client*)(GetAtIteratorFila2(&connected_clients));
	}
		
	//se tem dois conectados, disconecta um
	if(tempinfo->devices[1] == 1)
		tempinfo->devices[1] = 0;
	else //se não, remove a estrutura
		DeleteAtIteratorFila2(&connected_clients);
		
	pthread_mutex_unlock(&queue);
}



int insert_client(client *clientinfo){
	pthread_mutex_lock(&queue);
	
	//busca se já existe
	client *tempinfo;
	if(FirstFila2(&connected_clients) != 0){
		do{
			tempinfo = (client*)(GetAtIteratorFila2(&connected_clients));
			if (strcmp(tempinfo->userid, clientinfo->userid) == 0){ //mesmo id = já tem um logado
				if(tempinfo->devices[1] == 1) //terceiro cliente não pode logar
					return NOT_VALID;	
				else{
					tempinfo->devices[1] = 1;
					return ACCEPTED;
				}
			}
				
		}while(NextFila2(&connected_clients) != 0);
	}
	else{ //fila vazia pode inserir
		AppendFila2(&connected_clients, (void*)clientinfo);
		tempinfo->devices[0] = 1;
		return ACCEPTED;
	}
	
	//nao achou cliente na fila, insere no fim
	AppendFila2(&connected_clients, (void*)clientinfo);
	tempinfo->devices[0] = 1;
	
	pthread_mutex_unlock(&queue);

	return 1;
}


void run_client(void *conn_info)
{
	connection_info ci = *(connection_info*)conn_info;
	int socketfd = ci.socket_client;

    //inicializa mutex da fila de clientes
    if (pthread_mutex_init(&queue, NULL) != 0)
    {
        printf("\nMutex (queue) init failed\n");
        return;
    }

    //inicializa fila de clientes	
	if(CreateFila2(&connected_clients) != 0){
		printf("\nInit failed\n");
		return;
	}

	// VAI RECEBER O ID DO CLIENTE ANTES DE CRIAR O SYNC
	char clientid[MAXNAME];
	read(socketfd, buffer, BUFFER_SIZE);
	memcpy(clientid, buffer, MAXNAME);

	client *cli;

	// AQUI ELE TEM QUE ACEITAR O CLIENTE E ENVIAR MENSAGEM DE OK
	
	// fica esperando a segunda conexão do sync e quando recebe, cria outro socket/thread.
	int socket_connection, socket_sync;
	socklen_t sync_len;
	struct sockaddr_in serv_addr, sync_addr;

	int PORT = ci.port+1;
	
	if ((socket_connection = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening sync socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (bind(socket_connection, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding sync");
	
	listen(socket_connection, 1);
	sync_len = sizeof(struct sockaddr_in);
	
	if( (socket_sync = accept(socket_connection, (struct sockaddr *) &sync_addr, &sync_len)) )
	{
		int *new_sock;
		new_sock = malloc(1);
       	*new_sock = socket_sync;
		pthread_t sync_thread;
		pthread_create(&sync_thread, NULL, run_sync, (void*)new_sock);
		pthread_detach(sync_thread);

	}
	
	// terminou de criar a thread de sync. agora pode executar o loop normal.
	char buffer[BUFFER_SIZE];
	char message;
 
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
					disconnect_client(cli);
					pthread_exit();
					break;
				case DOWNLOAD:
					bzero(buffer, BUFFER_SIZE);
					read(socketfd, buffer, BUFFER_SIZE);
					send_file(buffer, socketfd);
					break;
				case UPLOAD:
					bzero(buffer, BUFFER_SIZE);
					read(socketfd, buffer, BUFFER_SIZE);
					receive_file(buffer, socketfd);
					break;
			}
		}
	}

	//free(socket_sync);

	close(socketfd);
}

void run_sync(void *socket_sync)
{
	char buffer[BUFFER_SIZE];
	int socketfd = *(int*)socket_sync;
	char message;

	while(1) {
		bzero(buffer, BUFFER_SIZE);

		message = read(socketfd, buffer, BUFFER_SIZE);

		if (message < 0) 
			printf("ERROR reading from socket");
		else 
		{
			memcpy(&message, buffer, 1);

			if(message == SYNC)
			{
				char client_id[MAXNAME];
				//recebe id do cliente. ---> VER SE NÃO É MELHOR ELE RECEBER ANTES???
				//pegar os dados do buffer
				read(socketfd, buffer, BUFFER_SIZE);
				memcpy(client_id, buffer, MAXNAME);
				
				bzero(buffer,BUFFER_SIZE);

				// pega cliente na lista de clientes e envia o mirror para o cliente.
				client *cli;

				memcpy(buffer, cli, sizeof(client));
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
						file_info *f = search_files(cli, fname);

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

						// deleta arquivo da pasta sync do server

						// deleta estrutura da lista de arquivos do cliente
					}
					else
						break;
				}
				
				// aí executa aqui o sync_server.
				sync_server(socketfd);
			}
		}
	}
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

	// TODO: função que recupera o cliente com client_mirror->userid da lista de clientes.
	client *cli;
	update_client(cli, home);
  
  	// pra cada arquivo do cliente:
  	int i;
  	for(i = 0; i < MAXFILES; i++) 
    {
      	if(strcmp(client_mirror.fileinfo[i].name, "") == 0)
           break;
    	else
        {
        	// arquivo existe no servidor?
			fi = search_files(cli, client_mirror.fileinfo[i].name);

			if(fi != NULL)		// arquivo existe no servidor
			{
				//verifica se o arquivo no cliente tem commit_created/modified > state do servidor.
				if(client_mirror.fileinfo[i].commit_modified > cli->current_commit)
				{
					//isso quer dizer que o arquivo no servidor é de um commit mais novo que o estado atual do cliente.
					// pede para o cliente mandar o arquivo
					bzero(buffer,BUFFER_SIZE);
					buffer[0] = DOWNLOAD;
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
				if(client_mirror.fileinfo[i].commit_modified > cli->current_commit)
				{
					//isso quer dizer que é um arquivo novo colocado no servidor em outro pc.
					// pede para o cliente mandar o arquivo
					bzero(buffer,BUFFER_SIZE);
					buffer[0] = DOWNLOAD;
					write(socketfd, buffer, BUFFER_SIZE);

					bzero(buffer,BUFFER_SIZE);

					memcpy(buffer, &client_mirror.fileinfo[i].name, MAXNAME);
					write(socketfd, buffer, BUFFER_SIZE);

					struct file_info f;
					//fica esperando receber struct
					bzero(buffer,BUFFER_SIZE);
					read(socketfd, buffer, BUFFER_SIZE);
					memcpy(&f, buffer, sizeof(struct file_info));
				
					//recebe arquivo
					//TODO

					//bota f na estrutura self
					insert_file_into_client_list(cli, f);
				}
			}
        }
    }

	//avança o estado de commit do cliente no servidor.
	cli->current_commit += 1;
}


int main(int argc, char *argv[])
{
	home[256] = "/home/";
	strcat(home, getlogin());
	strcat(home, "/server");

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
		
			pthread_create(&client_thread, NULL, run_client, (void*)new_sock);
		
			pthread_detach(client_thread);

			//free(new_sock);
		}
		
	}

	printf("saí do meu while (server)\n");
	
	return 0; 
}
