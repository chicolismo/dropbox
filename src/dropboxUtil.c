#include "../include/dropboxUtil.h"

int file_size(FILE *file){
    int size = -1;
    long int current_position;
    
    if(file) {
        current_position = ftell(file);
        fseek(file, 0L, SEEK_END);
        size = ftell(file);
        fseek(file, current_position, SEEK_SET);
    }
    
    printf("SENT FILE SIZE = %d\n", size);
    return size;
}

int write_file(char* file_name, int socket){
    int i;
    long int size = -1;
    unsigned char *sizeBuffer;
    FILE *file;
    char buffer;
    
    // Faz a leitura do tamanho do arquivo, convertendo para int
    sizeBuffer = (unsigned char*)&size;
    if(read(socket, sizeBuffer, 4) > 0) {
        printf("\nLi do socket\n");
        if((file = fopen(file_name, "w+")) == NULL) {
            printf("ERROR on openning file: file doesn't exist.\n");
        } else {
            for(i=0; i<size; i++){
                read(socket, (void*)&buffer, 1);
                fputc(buffer, file);
            }
        }
        fclose(file);
    }
    return size;
}

client map_sync_dir(char *home, char *login) 
{
	struct client c;

	//copia o login dropbox do usuário para userid
	strcpy(c.userid, login);
	c.logged_in = 1;

	//verifica se o diretorio sync_dir existe na home do usuario. se nao existir, cria.
	strcat(home, "/sync_dir_");
	strcat(home, login);

	struct stat st = {0};
	if (stat(home, &st) == -1)
		  mkdir(home, 0700);

	struct dirent *dir;	
	DIR *d;
	
	// lista todos os arquivos no diretorio sync_dir do usuario, colocando-os na estrutura self do cliente
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
	
	return c;
}

file_info search_files(client *client, char filename[MAXNAME])
{
	int i;
	for(i = 0; i < MAXFILES; i++)
	{
		if(strcmp(server_mirror.fileinfo[i].name, "") == 0)
			break;
		else
		{
			if(strcmp(filename, client->fileinfo[i].name) == 0)
				return client->fileinfo[i];
		}
	}
	return NULL;
}
