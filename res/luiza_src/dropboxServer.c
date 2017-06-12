//#include "../include/dropboxServer.h"
//#include "../include/dropboxClient.h"

#include "dropboxServer.h"
#include "dropboxUtil.h"
//#include "../../include/fila2.h"
//#include "fila2.h"

pthread_mutex_t queue;
//FILA2 connected_clients;
client connected_clients[256];
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

void list_files(file_info files[256]){
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

//-1 se nao achou o cliente i>=0 com o cliente na estrutura se achou
int return_client(char* user_name, client *new_client){
    int i;	
    pthread_mutex_lock(&queue);

    for(i=0;i<256;i++){
        if(connected_clients[i].logged_in == 1){
            if(strcmp(user_name, connected_clients[i].userid) == 0){
                pthread_mutex_unlock(&queue);
                return i;
            }
        }
    }
    pthread_mutex_unlock(&queue);
    return -1;
}




void disconnect_client(client *clientinfo){
    int i;
	pthread_mutex_lock(&queue);

	
	for(i=0;i<256;i++){
        if(connected_clients[i].logged_in == 1){
            if(strcmp(clientinfo->userid, connected_clients[i].userid) == 0){
                break;
            }
        }
    }
		
	//se tem dois conectados, disconecta um
	if(connected_clients[i].devices[1] == 1)
		connected_clients[i].devices[1] = 0;
	else //se não, desloga
		connected_clients[i].logged_in = 0;
		
	pthread_mutex_unlock(&queue);
}



int insert_client(client *clientinfo){
	pthread_mutex_lock(&queue);
    int i;

    for(i=0;i<256;i++){
        if(connected_clients[i].logged_in == 1)
		{
            if(strcmp(clientinfo->userid, connected_clients[i].userid) == 0)
			{
				if(connected_clients[i].devices[1] == 0)
				{
		            connected_clients[i].devices[1] = 1; //ja esta conectado, ativar segundo device
					pthread_mutex_unlock(&queue);
		            return ACCEPTED;
				}
				else
				{
					pthread_mutex_unlock(&queue);
					return NOT_VALID;
				}
            }
        }
		else
		{
			memcpy(&connected_clients[i], clientinfo, sizeof(client));
			//connected_clients[i] = *clientinfo;
			connected_clients[i].devices[0] = 1;
			pthread_mutex_unlock(&queue);
			return ACCEPTED;
		}
    }

	pthread_mutex_unlock(&queue);

	return NOT_VALID;
}


void* run_client(void *conn_info)
{
	char buffer[BUFFER_SIZE];
	char message;
	connection_info ci = *(connection_info*)conn_info;
	int socketfd = ci.socket_client;

	printf("entrei no run client\n");

	// VAI RECEBER O ID DO CLIENTE ANTES DE CRIAR O SYNC
	// AQUI ELE TEM QUE ACEITAR O CLIENTE E ENVIAR MENSAGEM DE OK
	char clientid[MAXNAME];

	bzero(buffer, BUFFER_SIZE);
	read(socketfd, buffer, BUFFER_SIZE);
	memcpy(clientid, buffer, MAXNAME);

	client *cli = malloc(sizeof(client));
	init_client(cli, home, clientid);
	
	if(insert_client(cli) == ACCEPTED)
	{
		bzero(buffer, BUFFER_SIZE);
		buffer[0] = 'A';		
		write(socketfd, buffer, BUFFER_SIZE);
	}
	else
	{
		bzero(buffer, BUFFER_SIZE);
		buffer[0] = 'N';
		write(socketfd, buffer, BUFFER_SIZE);
		
		close(socketfd);
		pthread_exit(0);
	}

	printf("EEEEEEEEE\n");
	
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
					pthread_exit(0);
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

void* run_sync(void *socket_sync)
{
	printf("running sync!\n");
	char buffer[BUFFER_SIZE];
	int socketfd = *(int*)socket_sync;
	char message;

	while(1) {
		printf("while do sync\n");
		bzero(buffer, BUFFER_SIZE);
		message = read(socketfd, buffer, BUFFER_SIZE);

		if (message < 0) 
			printf("ERROR reading from socket");
		else 
		{
			message = buffer[0];

			if(message == SYNC)
			{
				printf("message == sync\n");
				char client_id[MAXNAME];
				//recebe id do cliente. ---> VER SE NÃO É MELHOR ELE RECEBER ANTES???
				//pegar os dados do buffer
				bzero(buffer, BUFFER_SIZE);
				read(socketfd, buffer, BUFFER_SIZE);
				memcpy(client_id, buffer, MAXNAME);

				printf("buff: %s\n", buffer);
				
				// pega cliente na lista de clientes e envia o mirror para o cliente.
				client *cli = malloc(sizeof(client));
				int cliindex = return_client(client_id, cli); 

				update_client(&(connected_clients[cliindex]), home);
				client c = connected_clients[cliindex];
			
				printf("got client! %s\n", c.userid);

				bzero(buffer,BUFFER_SIZE);
				memcpy(buffer, &c, sizeof(client));
				write(socketfd, buffer, BUFFER_SIZE);

				printf("ok\n");

				// agora fica em um while !finished, fica recebendo comandos de download/delete
				while(1)
				{	
					printf("while dos arquivos\n");
					
					char command;
					char fname[MAXNAME];

					bzero(buffer,BUFFER_SIZE);
					read(socketfd, buffer, BUFFER_SIZE);
					command = buffer[0];

					printf("command: %c\n", command);

					if(command == DOWNLOAD)
					{
						printf("downloading\n");
						// recebe nome do arquivo
						bzero(buffer,BUFFER_SIZE);
						read(socketfd, buffer, BUFFER_SIZE);
						memcpy(fname, buffer, MAXNAME);
						
						// procura arquivo
						int index = search_files(&(connected_clients[cliindex]), fname);
						file_info f;

						if(index >= 0)
							f = connected_clients[cliindex].fileinfo[index];

						// manda struct
						bzero(buffer,BUFFER_SIZE);
						memcpy(buffer, &f, sizeof(file_info));
						write(socketfd, buffer, BUFFER_SIZE);

						// receive file funciona com full path
						char fullpath[256];
						strcpy(fullpath, home);
						strcat(fullpath, "/sync_dir_");
						strcat(fullpath, connected_clients[cliindex].userid);
						strcat(fullpath, "/");
						strcat(fullpath, f.name);
						strcat(fullpath, ".");
						strcat(fullpath, f.extension);

						// manda arquivo				
						send_file(fullpath, socketfd);
					}
					else if(command == DELETE)
					{
						bzero(buffer,BUFFER_SIZE);

						// recebe nome do arquivo
						read(socketfd, buffer, BUFFER_SIZE);
						memcpy(fname, buffer, MAXNAME);

						// procura arquivo
						int index = search_files(cli, fname);
						file_info f;

						if(index >= 0)
							f = connected_clients[cliindex].fileinfo[index];

						// deleta arquivo da pasta sync do server
						char fullpath[256];
						strcpy(fullpath, home);
						strcat(fullpath, "/sync_dir_");
						strcat(fullpath, connected_clients[cliindex].userid);
						strcat(fullpath, "/");
						strcat(fullpath, f.name);
						strcat(fullpath, ".");
						strcat(fullpath, f.extension);
			
						remove_file(fullpath);

						// deleta estrutura da lista de arquivos do cliente
						delete_file_from_client_list(&(connected_clients[cliindex]), fname);
					}
					else
						break;
				}
				
				printf("saí do while\n");
				// aí executa aqui o sync_server.
				sync_server(socketfd);
			}
		}
	}
}

void sync_server(int socketfd)
{

	printf("entrei no sync_server\n");
	struct client client_mirror;
	char buffer[BUFFER_SIZE];

	//server fica esperando o cliente enviar seu mirror
	bzero(buffer,BUFFER_SIZE);
	read(socketfd, buffer, BUFFER_SIZE);
	memcpy(&client_mirror, buffer, sizeof(struct client));

	printf("mirror %s\n", client_mirror.userid);

	// TODO: função que recupera o cliente com client_mirror->userid da lista de clientes.
	client *cli = malloc(sizeof(client));
	int cliindex = return_client(client_mirror.userid, cli); 

	update_client(&(connected_clients[cliindex]), home);
	client c = connected_clients[cliindex];

	printf("%s\n", c.userid);
  
  	// pra cada arquivo do cliente:
  	int i;
  	for(i = 0; i < MAXFILES; i++) 
    {
		printf("files\n");
		printf("file: %s\n", client_mirror.fileinfo[i].name);
      	if(strcmp(client_mirror.fileinfo[i].name, "\0") == 0)
		{
			printf("break\n");
           break;
		}
    	else
        {
        	// arquivo existe no servidor?
			int index = search_files(&(connected_clients[cliindex]), client_mirror.fileinfo[i].name);
			
			if(index >= 0)		// arquivo existe no servidor
			{
				printf("arquivo existe aqui\n");
				//verifica se o arquivo no cliente tem commit_created/modified > state do servidor.
				if(client_mirror.fileinfo[i].commit_modified >= connected_clients[cliindex].current_commit)
				{
					//isso quer dizer que o arquivo no servidor é de um commit mais novo que o estado atual do cliente.
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
				
					// receive file funciona com full path
					char fullpath[256];
					strcpy(fullpath, home);
					strcat(fullpath, "/sync_dir_");
					strcat(fullpath, connected_clients[cliindex].userid);
					strcat(fullpath, "/");
					strcat(fullpath, f.name);
					strcat(fullpath, ".");
					strcat(fullpath, f.extension);
			
					//recebe arquivo
					receive_file(fullpath, socketfd);

					// atualiza na estrutura do cliente no servidor.
					memcpy(&connected_clients[cliindex].fileinfo[index], &f, sizeof(file_info));
				}
			}
			else				// arquivo não existe no servidor
			{
				printf("arquivo não tem no servidor\n");
				// verifica se o arquivo no cliente tem um commit_modified > state do servidor
				if(client_mirror.fileinfo[i].commit_modified >= connected_clients[cliindex].current_commit)
				{
					printf("dentro do if\n");
					//isso quer dizer que é um arquivo novo colocado no servidor em outro pc.
					// pede para o cliente mandar o arquivo
					bzero(buffer,BUFFER_SIZE);
					buffer[0] = DOWNLOAD;
					write(socketfd, buffer, BUFFER_SIZE);

					printf("mandei dw\n");

					bzero(buffer,BUFFER_SIZE);
					memcpy(buffer, client_mirror.fileinfo[i].name, MAXNAME);
					write(socketfd, buffer, BUFFER_SIZE);

					printf("mandei arquivo\n");

					struct file_info f;
					//fica esperando receber struct
					bzero(buffer,BUFFER_SIZE);
					read(socketfd, buffer, BUFFER_SIZE);
					memcpy(&f, buffer, sizeof(struct file_info));
				
					// receive file funciona com full path
					char fullpath[256];
					strcpy(fullpath, home);
					strcat(fullpath, "/sync_dir_");
					strcat(fullpath, connected_clients[cliindex].userid);
					strcat(fullpath, "/");
					strcat(fullpath, f.name);
					strcat(fullpath, ".");
					strcat(fullpath, f.extension);
		
					printf("receiving path: %s\n", fullpath);
			
					//recebe arquivo
					receive_file(fullpath, socketfd);

					//bota f na estrutura self
					insert_file_into_client_list(&(connected_clients[cliindex]), f);
				}
			}
        }
    }

	//avança o estado de commit do cliente no servidor.
	connected_clients[cliindex].current_commit += 1;

	// avisa que acabou o seu sync.
	bzero(buffer, BUFFER_SIZE);
	buffer[0] = SYNC_END;
	write(socketfd, buffer, BUFFER_SIZE);
}


int main(int argc, char *argv[])
{
    int i;
	strcpy(home,"/home/");
	strcat(home, getlogin());
	strcat(home, "/server");

	struct stat st;
	if (stat(home, &st) != 0) {
		  mkdir(home, 0777);
	}
		
	for(i=0;i<256;i++){
        connected_clients[i].logged_in = 0;
    }   

	int socket_connection, socket_client;
	socklen_t client_len;
	struct sockaddr_in serv_addr, client_addr;


    //inicializa mutex da fila de clientes
    if (pthread_mutex_init(&queue, NULL) != 0)
    {
        printf("\nMutex (queue) init failed\n");
        return 0;
    }

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

			connection_info *ci = malloc(sizeof(connection_info));
			ci->socket_client = socket_client;
			ci->port = PORT;
		
			pthread_create(&client_thread, NULL, run_client, (void*)ci);
		
			pthread_detach(client_thread);

			//free(new_sock);
		}
		
	}

	printf("saí do meu while (server)\n");
	
	return 0; 
}
