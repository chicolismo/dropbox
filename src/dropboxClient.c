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
	struct stat st = {0};

	// se o sync_dir nao existir, cria-lo
	if (stat("/home/luiza/sync_dir", &st) == -1) {
		mkdir("/home/luiza/sync_dir", 0700);
	}

	// cliente deve ver se existem arquivos do servidor para espelhar na pasta sync_dir

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

	struct client self;
	struct dirent *dir;
	struct stat st = {0};
	DIR *d;

	strcpy(self.userid, argv[1]);
	self.logged_in = 0;
	char home[256] = "/home/";
	strcat(home, getlogin());
	strcat(home, "/sync_dir");

	if (stat(home, &st) == -1)
		  mkdir(home, 0700);


	d = opendir(home);
	if (d)
	{
	  int i = 0;
	  while ((dir = readdir(d)) != NULL)
	  {
		  struct file_info fi;

		  //TEM QUE PARSEAR O NOME DO ARQUIVO
		  strcpy(fi.name, d->d_name);
		  strcpy(fi.extension, d->d_name);

		  // pegar ultima data de modificação do arquivo
		  struct stat attrib;
		  stat(d->d_name, &attrib);
		  strftime(fi.last_modified, MAXNAME, "%d-%m-%y", gmtime(&(attrib.st_ctime)));
		  // strftime(date, 20, "%d-%m-%y", localtime(&(attrib.st_ctime)));
		  fi.size = attrib.st_siz;

		  self.fileinfo[i] = fi;
		  i++;
	  }
	  closedir(d);
	}
	// conecta este cliente com o servidor, que criará uma thread para administrá-lo
	socketfd = connect_server(argv[2], atoi(argv[3]));

	//send to server
	send(server, self, sizeof(struct client), 0);
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
