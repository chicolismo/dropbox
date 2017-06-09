#include "../include/dropboxClient.h"
#include "../include/dropboxUtil.h"

struct client self;

void get_file(char* file, int server_socket){
    char buffer[256];

	// Chama funcao que escreve arquivo
    if(write_file(file, server_socket) < 0){
        printf("ERROR on openning file: file doesn't exist.\n");
    }
    else {
		strcpy(buffer, "SUCCESS: file received.\n");
        write(server_socket, buffer, sizeof(buffer));
        printf("File %s downloaded.\n", file);
    }
}

void send_file(char *file, int server_socket) {
	FILE *fp;
	int offset;
	long int size;
	char buffer, server_answer[256];
	unsigned char* bufferSize;

	if(!(fp = fopen(file, "r"))) {
		printf("ERROR on openning file: file doesn't exist.\n");
	} else {
		size = file_size(fp);
		bufferSize = (unsigned char*) &size;

		// Se enviar o tamanho do arquivo
		if(write(server_socket, (void*)bufferSize, 4) > 0) {
			// Prepara envio do arquivo
			offset = 0;
			while(offset < size) {
				buffer = fgetc(fp);
				// Envia 1 byte do arquivo
				if(write(server_socket, (void*)&buffer, 1) < 0)
					break;
				offset++;
			}
		}

		printf("File %s uploaded.\n", file);
		fclose(fp);

		read(server_socket, server_answer, 256);
		printf("Server: %s\n", server_answer);
	}
}



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
	struct file_info fi;
	
  	// fica esperando o servidor enviar o espelho de todos os arquivos que estao la
    recv(socketfd, server_mirror, sizeof(struct client), 0);
  
  	// recebeu. agora vai comparar com os arquivos dele
  	int i;
  	for(i = 0; i < MAXFILES; i++) 
    {
      	if(strcmp(server_mirror.fileinfo[i].name, "") == 0)
           break;
    	else
        {
        	// fazer uma função que busca dentro do vetor file_info um struct file_info com o mesmo nome.
			fi = search_files(&self, server_mirror.fileinfo[i].name);

			if(fi != NULL)
			{
				if(file_more_recent_than(server_mirror.fileinfo[i], fi)
				{
					//update file in client directory
					//download file
				}
          	}
			else
			{
				//download file
			}
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
	self = map_sync_dir(home, argv[1]);

	// conecta este cliente com o servidor, que criará uma thread para administrá-lo
	socketfd = connect_server(argv[2], atoi(argv[3]));

	//send to server
	send(socketfd, self, sizeof(struct client), 0);

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
