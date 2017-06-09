#include "../include/dropboxServer.h"
#include "../include/dropboxUtil.h"


void receive_file(char* file, int client_socket){
    char buffer[256];
    
    // Chama funcao que escreve arquivo
    int file_size = write_file(file, client_socket);
    printf("RECEIVED FILE SIZE = %d\n", file_size);
    
    if(file_size < 0){
        strcpy(buffer, "ERROR: file writing failed.");
        write(client_socket, buffer, sizeof(buffer));
        printf("ERROR on openning file: file doesn't exist.\n");
    }
    else {
        strcpy(buffer, "SUCCESS: file received.\n");
        write(client_socket, buffer, sizeof(buffer));
        printf("File %s downloaded.\n", file);
    }
}

void send_file(char *file, int client_socket) {
    FILE *fp;
    int offset;
    long int size;
    char buffer, server_answer[256];
    unsigned char* sizeBuffer;
    
    if(!(fp = fopen(file, "r"))) {
        printf("ERROR on openning file: file doesn't exist.\n");
    } else {
        size = file_size(fp);
        sizeBuffer = (unsigned char*) &size;
        
        // Se enviar o tamanho do arquivo
        if(write(client_socket, (void*)sizeBuffer, 4) > 0) {
            // Prepara envio do arquivo
            offset = 0;
            while(offset < size) {
                buffer = fgetc(fp);
                // Envia 1 byte do arquivo
                if(write(client_socket, (void*)&buffer, 1) < 0)
                    break;
                offset++;
            }
        }
        
        printf("File %s uploaded.\n", file);
        fclose(fp);
        
        read(client_socket, server_answer, 256);
        printf("CLIENT RESPONSE: %s\n", server_answer);
    }
}

void run_thread(void *socket_client)
{
	char buffer[BUFFER_SIZE];
	int socketfd = *(int*)socket_client;
	int message;
	printf("i created a thread\n");
	

	while(1) {
		bzero(buffer, BUFFER_SIZE);
		message = read(socketfd, buffer, BUFFER_SIZE);
		if (message < 0) 
			printf("ERROR reading from socket");
		else {
			printf("Here is the message: %s\n", buffer);
			// write
			message = write(socketfd,"I got your message", 18);
		}
		
	}

	free(socket_client);

	close(socketfd);
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
