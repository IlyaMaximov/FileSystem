#ifndef MINIFS_CLIENT_H
#define MINIFS_CLIENT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <libsync.h>

typedef struct Client Client;
struct Client {
    int sock;
    struct sockaddr_in addr;

    char* request;
    char* response;
    uint32_t request_len;
    uint32_t response_len;
    u_int32_t request_cap;
    u_int32_t response_cap;
};

Client FS_CLIENT;

void initClient();

void clearClientData();

void sendToServer(char* request);

void sendRequestToServer();

char* receiveFromServer();

void deleteClient();

#endif //MINIFS_CLIENT_H
