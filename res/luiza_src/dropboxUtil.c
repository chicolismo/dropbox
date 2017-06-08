#include "../include/dropboxUtil.h"

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
		  	strftime(fi.last_modified, MAXNAME, "%d-%m-%Y-%H-%M-%S", gmtime(&(attrib.st_mtime)));
		  	// strftime(date, 20, "%d-%m-%y", localtime(&(attrib.st_ctime)));
		  	fi.size = attrib.st_siz;

			//leitura do arquivo está sendo feita neste commit. colocar o commit atual do cliente no arquivo

			fi.commit_modified = client->current_commit;

			file_info *f = search_files(client, name);
			if(f != NULL)
			{
				// arquivo já existe na estrutura
				// se a data de modificação do arquivo que eu to lendo agora for mais recente que o que ja tava na estrutura self, sobrescrever.
				if(file_more_recent_than(fi, f))
					*f = fi;
			}
			else
			{
				//arquivo não está na estrutura, adicionar.
				insert_file_into_client_list(client, fi);
			}
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

void delete_file_from_client_list(client *client, char filename[MAXNAME])
{
	return;
}

// retorna 1 se f1 é mais recente que f2. retorna 0  caso contrário
int file_more_recent_than(file_info f1, file_info f2)
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

