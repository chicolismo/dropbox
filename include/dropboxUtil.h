//#include "fila2.h"
#include <dirent.h> 

#define BUFFER_SIZE 256
#define MAXNAME 256
#define MAXFILES 30

//flags de controle
#define EXIT 'e'
#define SYNC 's'
#define DOWNLOAD 'd'
#define UPLOAD 'u'
#define DELETE 'x'

struct file_info {
	char name[MAXNAME];
	char extension[MAXNAME];
	char last_modified[MAXNAME];
	int size;
};

struct client {
	int devices[2];
	char userid[MAXNAME];
	struct file_info fileinfo[MAXFILES];
	int logged_in;
} client;

int file_size(FILE *file);
int write_file(char* file_name, int socket);
client map_sync_dir(char *home, char *login);
file_info search_files(client *client, char filename[MAXNAME]);
