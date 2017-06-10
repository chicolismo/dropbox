//#include "fila2.h"
#include <dirent.h> 
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define BUFFER_SIZE 256
#define MAXNAME 256
#define MAXFILES 30
#define SLEEP 30

//flags de controle
#define EXIT "e"
#define SYNC "s"
#define DOWNLOAD "d"
#define UPLOAD "u"
#define DELETE "x"

typedef struct file_info {
	char name[MAXNAME];
	char extension[MAXNAME];
	char last_modified[MAXNAME];
	int size;
	int commit_modified;
} file_info;

typedef struct client {
	int devices[2];
	char userid[MAXNAME];
	struct file_info fileinfo[MAXFILES];
	int logged_in;
	int current_commit;
} client;

typedef struct connection_info {
	int port;
	int socket_client; 
} connection_info;	

void init_client(client *client, char *home, char *login);
void update_client(client *client, char *home);
file_info* search_files(client *client, char filename[MAXNAME]);
void insert_file_into_client_list(client *client, file_info fileinfo);
void delete_file_from_client_list(client *client, char filename[MAXNAME]);
int file_more_recent_than(file_info *f1, file_info *f2);
