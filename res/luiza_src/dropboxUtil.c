#include "../include/dropboxUtil.h"

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

void update_client(client *client, char *home)
{
	//setup do nome do diretório em que ele precisa procurar.
	char sync_dir*;
	strcat(sync_dir,home);
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
			
			// d_name é o nome do arquivo sem o resto do path. ta de boasssss
			char *name = strtok(d->d_name, ".");
			char *extension = atoi(strtok(NULL, "."));
		
		  	strcpy(fi.name, name);
		  	strcpy(fi.extension, extension);

		  	// pegar ultima data de modificação do arquivo
			//STAT É CHAMADO COM FULL PATH, TEM QUE CONCATENAR
			char *fullpath;
			strcat(fullpath, sync_dir);
			strcat(fullpath, name);
			strcat(fullpath, ".");
			strcat(fullpath, extension);

		  	struct stat attrib;
		  	stat(fullpath, &attrib);
		  	strftime(fi.last_modified, MAXNAME, "%d-%m-%y", gmtime(&(attrib.st_ctime)));
		  	// strftime(date, 20, "%d-%m-%y", localtime(&(attrib.st_ctime)));
		  	fi.size = attrib.st_siz;

		  	self.fileinfo[i] = fi;
		  	i++;
	  }
	  closedir(d);
	}

}



file_info* search_files(client *client, char filename[MAXNAME])
{
	int i;
	for(i = 0; i < MAXFILES; i++)
	{
		if(strcmp(server_mirror.fileinfo[i].name, "") == 0)
			break;
		else
		{
			if(strcmp(filename, client->fileinfo[i].name) == 0)
				return &(client->fileinfo[i]);
		}
	}
	return NULL;
}

void insert_file_into_client_list(client *client, file_info fileinfo)
{
	int i;
	for(i = 0; i < MAXFILES; i++)
	{
		if(strcmp(server_mirror.fileinfo[i].name, "") == 0)
		{
			//achou o fim da lista.
			if(i == MAXFILES-1)
			{
				//se é a última posição possível, só colocar o arquivo na última posição e fim de papo
				client->fileinfo[i] = fileinfo;
			}
			else
			{
				//ainda há posições disponíveis. é preciso marcar o fim da lista com a string vazia.
				client->fileinfo[i] = fileinfo;
				client->fileinfo[i+1].name = "";
			}
			break;
		}
	}
}

