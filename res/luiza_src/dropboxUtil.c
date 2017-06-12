//#include "../include/dropboxUtil.h"
#include "dropboxUtil.h"

void init_client(client *client, char *home, char *login)
{
	//inicializa estrutura self do cliente
	strcpy(client->userid, login);

	//inicializa todos os nomes de arquivo como empty string, preparando para a função update client
	int i;
	for(i = 0; i < MAXFILES; i++)
		strcpy(client->fileinfo[i].name, "\0");
	
	//verifica se o diretorio sync_dir existe na home do usuario. se nao existir, cria.
	char sync_dir[256];
	strcpy(sync_dir, home);
	strcat(sync_dir, "/sync_dir_");
	strcat(sync_dir, login);


	struct stat st;
	if (stat(sync_dir, &st) != 0) {
		  mkdir(sync_dir, 0777);
	}

	client->logged_in = 1;
	client->current_commit = 0;
	client->devices[0] = 1;

	update_client(client, home);
}

void update_client(client *client, char *home)
{
	//setup do nome do diretório em que ele precisa procurar.
	char sync_dir[256];
	strcpy(sync_dir,home);
	strcat(sync_dir,"/sync_dir_");
	strcat(sync_dir,client->userid);
 
	struct dirent *dir;	
	DIR *d;
	
	// lista todos os arquivos no diretorio sync_dir do usuario, colocando-os na estrutura self do cliente
	d = opendir(sync_dir);
	if (d)
	{
	  int i = 0;
	  while ((dir = readdir(d)) != NULL)
	  {
			struct file_info fi;
			
			if(strcmp(dir->d_name, "..") == 0)
				break;

			// d_name é o nome do arquivo sem o resto do path. ta de boasssss
			char *name = strtok(dir->d_name, ".");
			char *extension = strtok(NULL, ".");
		
		  	strcpy(fi.name, name);
		  	strcpy(fi.extension, extension);

		  	// pegar ultima data de modificação do arquivo
			//STAT É CHAMADO COM FULL PATH, TEM QUE CONCATENAR
			char fullpath[256];
			strcpy(fullpath, sync_dir);
			strcat(fullpath, name);
			strcat(fullpath, ".");
			strcat(fullpath, extension);

		  	struct stat attrib;
		  	stat(fullpath, &attrib);
		  	strftime(fi.last_modified, MAXNAME, "%d-%m-%Y-%H-%M-%S", gmtime(&(attrib.st_mtime)));
		  	// strftime(date, 20, "%d-%m-%y", localtime(&(attrib.st_ctime)));
		  	fi.size = (int)attrib.st_size;

			//leitura do arquivo está sendo feita neste commit. colocar o commit atual do cliente no arquivo

			fi.commit_modified = client->current_commit;

			int index = search_files(client, name);

			if(index > 0) // arquivo já existe na estrutura
			{
				file_info f = client->fileinfo[index];
				
				// se a data de modificação do arquivo que eu to lendo agora for mais recente que o que ja tava na estrutura self, sobrescrever.
				if(file_more_recent_than(&fi, &f))
					client->fileinfo[index] = fi;
				else
				{
					//arquivo não está na estrutura, adicionar.
					insert_file_into_client_list(client, fi);
				}
			}
		  	i++;
	  }
	  closedir(d);
	}
}

int search_files(client *client, char filename[MAXNAME])
{
	int i;
	for(i = 0; i < MAXFILES; i++)
	{
		if(strcmp(client->fileinfo[i].name,"\0") != 0)
		{
			if(strcmp(filename, client->fileinfo[i].name) == 0)
				return i;
		}
	}
	return -1;
}

void insert_file_into_client_list(client *client, file_info fileinfo)
{
	int i;
	for(i = 0; i < MAXFILES; i++)
	{
		// bota na primeira posição livre que achar.
		if(strcmp(client->fileinfo[i].name,"\0") == 0)
		{
			client->fileinfo[i] = fileinfo;
			break;
		}
	}
}

void delete_file_from_client_list(client *client, char filename[MAXNAME])
{
	int index = search_files(client, filename);
	if(index > 0)
		strcpy(client->fileinfo[index].name,"\0");
}

// retorna 1 se f1 é mais recente que f2. retorna 0  caso contrário
int file_more_recent_than(file_info *f1, file_info *f2)
{
	/* DATE FORMAT
		"%d-%m-%Y-%H-%M-%S"
		31-01-2001-13-14-23
	*/

	//copia strings pra não serem destruídas pelo strtok caso seja por ref.
	char *f1_lm, *f2_lm;
	strcpy(f1_lm, f1->last_modified);
	strcpy(f2_lm, f2->last_modified);

	int f1_day = atoi(strtok(f1_lm, "-"));
	int f1_month = atoi(strtok(NULL, "-"));
	int f1_year = atoi(strtok(NULL, "-"));
	int f1_hour = atoi(strtok(NULL, "-"));
	int f1_minutes = atoi(strtok(NULL, "-"));
	int f1_seconds = atoi(strtok(NULL, "-"));

	int f2_day = atoi(strtok(f2_lm, "-"));
	int f2_month = atoi(strtok(NULL, "-"));
	int f2_year = atoi(strtok(NULL, "-"));
	int f2_hour = atoi(strtok(NULL, "-"));
	int f2_minutes = atoi(strtok(NULL, "-"));
	int f2_seconds = atoi(strtok(NULL, "-"));

	if(f1_year > f2_year)
		return 1;
	else if (f2_year > f1_year)
		return 0;
	else
	{
		if(f1_month > f2_month)
			return 1;
		else if (f2_month > f1_month)
			return 0;
		else
		{
			if(f1_day > f2_day)
				return 1;
			else if(f2_day > f1_day)
				return 0;
			else 
			{
				if(f1_hour > f2_hour)
					return 1;
				else if(f2_hour > f1_hour)
					return 0;
				else
				{
					if(f1_minutes > f2_minutes)
						return 1;
					else if(f2_minutes > f1_minutes)
						return 0;
					else 
					{
						if(f1_seconds > f2_seconds)
							return 1;
						else
							return 0;
					}
				}
			}
		}
	}
}





void receive_file(char* file_name, int client_socket)
{
    char buffer[256], char_buffer[1];
    int error;
    FILE *fp;

    //printf("%s", file_name);
    //open file
    fp = fopen(file_name, "w");


    bzero(char_buffer, 1);
    read(client_socket, char_buffer, 1);
    //fputc(char_buffer[0], fp);
    while(char_buffer[0] != EOF)
    {
        fputc(char_buffer[0], fp);
        bzero(char_buffer, 1);
        read(client_socket, char_buffer, 1);
        
	}
    
}


void send_file(char *file, int server_socket) 
{
    char buffer[256], char_buffer, cb[1];
    int error;
    FILE *fp;

    //printf("%s", file);
    //abrir arquivo para leitura
    fp = fopen(file, "r");

    //passar char a char
    char_buffer =  fgetc(fp);
    while(char_buffer != EOF)
    {    
        bzero(cb, 1);
        cb[0] = char_buffer;
        write(server_socket, cb, 1);
        char_buffer =  fgetc(fp);
    }

    bzero(cb, 1);
    cb[0] = char_buffer;
    write(server_socket, cb, 1);
}




void remove_file(char *filename)
{
        unlink(filename);
}




/*
void receive_file(int recv_socket){
    char buffer[256], file_name[256], char_buffer[1];
    int error;
    FILE *fp;

    //receiving file name    
    bzero(buffer, 256);
    bzero(file_name, 256);
    error = read(recv_socket, buffer, 256);
    if (error < 0){printf("ERROR GETTING FILE NAME"); return;}
    strncpy(file_name, buffer, strlen(buffer));

    //open file
    fp = fopen(file_name, "w");


    bzero(char_buffer, 1);
    read(recv_socket, char_buffer, 1);
    //fputc(char_buffer[0], fp);
    while(char_buffer[0] != EOF)
    {
        fputc(char_buffer[0], fp);
        bzero(char_buffer, 1);
        read(recv_socket, char_buffer, 1);
        
	}
    
}*/



/*
void send_file(char *file, int sendto_socket) {
    char buffer[256], char_buffer, cb[1];
    int error;
    FILE *fp;

    //sending file name
    bzero(buffer, 256);
    sprintf(buffer, "%s", file);
    error = write(sendto_socket, buffer, strlen(buffer));
    if (error < 0) {printf("ERROR writing to socket\n"); return;}

    //abrir arquivo para leitura
    fp = fopen(file, "r");

    //passar char a char
    char_buffer =  fgetc(fp);
    while(char_buffer != EOF)
    {    
        bzero(cb, 1);
        cb[0] = char_buffer;
        write(sendto_socket, cb, 1);
        char_buffer =  fgetc(fp);
    }

    bzero(cb, 1);
    cb[0] = char_buffer;
    write(sendto_socket, cb, 1);
}*/


