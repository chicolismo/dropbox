//#include "fila2.h"
#include <dirent.h> 

#define BUFFER_SIZE 256
#define MAXNAME 256
#define MAXFILES 30

struct file_info {
	char name[MAXNAME];
	char extension[MAXNAME];
	char last_modified[MAXNAME];
	int size;
	int commit_modified;
};

struct client {
	int devices[2];
	char userid[MAXNAME];
	struct file_info fileinfo[MAXFILES];
	int logged_in;
	int current_commit;
} client;

client map_sync_dir(char *home, char *login);
file_info* search_files(client *client, char filename[MAXNAME]);
void insert_file_into_client_list(client *client, file_info fileinfo);
