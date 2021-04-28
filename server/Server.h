#ifndef MINIFS_SERVER_H
#define MINIFS_SERVER_H

#include <stdlib.h>
#include <signal.h>
#include <sys/syslog.h>
#include <netinet/in.h>
#include "../console/Console.h"

typedef struct Server Server;
struct Server {
    int sock;
    int listener;
    struct sockaddr_in addr;

    char* request;
    char* response;
    uint32_t request_len;
    uint32_t response_len;
    u_int32_t request_cap;
    u_int32_t response_cap;
};

Server FS_SERVER;

void demonize();

void initServer();

void acceptClient();

void clearServerData();

void sendToClient(char* response);

void sendResponseToClient();

char* receiveFromClient();

void destructServer();

#endif //MINIFS_SERVER_H
