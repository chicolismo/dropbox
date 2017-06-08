#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MIN_ARG 1
#define MAX_CONNECTIONS 5

void sync_server();
void receive_file(char *file, int client_socket);
void send_file(char *file, int client_socket);
