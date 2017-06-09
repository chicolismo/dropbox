#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h> 

#define MIN_ARG 3

//AEHOOOOOOOOOOOOO

int connect_server(char *host, int port);
void sync_client();
void send_file(char *file, int server_socket);
void get_file(char *file, int server_socket);
void close_connection();
