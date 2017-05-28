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
