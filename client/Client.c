#include "Client.h"

#define MESSAGE_MAX_SIZE 4096

void initClient() {
    FS_CLIENT.sock = socket(AF_INET, SOCK_STREAM, 0);
    FS_CLIENT.addr.sin_family = AF_INET;
    FS_CLIENT.addr.sin_port = htons(3425);
    FS_CLIENT.addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    FS_CLIENT.request_cap = 2 * MESSAGE_MAX_SIZE;
    FS_CLIENT.response_cap = 2 * MESSAGE_MAX_SIZE;
    FS_CLIENT.request = calloc(FS_CLIENT.request_cap, 1);
    FS_CLIENT.response = calloc(FS_CLIENT.response_cap, 1);
    FS_CLIENT.request_len = 0;
    FS_CLIENT.response_len = 0;

    if (connect(FS_CLIENT.sock, (struct sockaddr *) &FS_CLIENT.addr, sizeof(FS_CLIENT.addr)) == -1) {
        perror("Client connect error");
        exit(EXIT_FAILURE);
    }
}

void clearClientData() {
    if (FS_CLIENT.request_cap > 2 * MESSAGE_MAX_SIZE) {
        FS_CLIENT.request = realloc(FS_CLIENT.request, 2 * MESSAGE_MAX_SIZE);
    }
    if (FS_CLIENT.response_cap > 2 * MESSAGE_MAX_SIZE) {
        FS_CLIENT.response = realloc(FS_CLIENT.response, 2 * MESSAGE_MAX_SIZE);
    }

    FS_CLIENT.request_cap = 2 * MESSAGE_MAX_SIZE;
    FS_CLIENT.response_cap = 2 * MESSAGE_MAX_SIZE;

    FS_CLIENT.request_len = 0;
    FS_CLIENT.response_len = 0;

    memset(FS_CLIENT.request, 0, FS_CLIENT.request_cap);
    memset(FS_CLIENT.response, 0, FS_CLIENT.response_cap);
}

// This is an "abstraction" function
// It construct FS_CLIENT.request
void sendToServer(char* new_request) {
    uint32_t request_len = strlen(new_request);
    if (FS_CLIENT.request_len + request_len >= FS_CLIENT.request_cap) {
        FS_CLIENT.request_cap = FS_CLIENT.request_len + request_len + 1;
        FS_CLIENT.request = realloc(FS_CLIENT.request, FS_CLIENT.request_cap);
    }
    memcpy(FS_CLIENT.request + FS_CLIENT.request_len, new_request, request_len);
    FS_CLIENT.request_len += request_len;
}

void sendRequestToServer() {
    char* request = FS_CLIENT.request;
    uint32_t request_len = FS_CLIENT.request_len;

    while (request_len > 0) {

        size_t message_len = request_len;
        if (message_len > MESSAGE_MAX_SIZE) {
            message_len = MESSAGE_MAX_SIZE;
        }

        ssize_t send_bytes = send(FS_CLIENT.sock, request, message_len, 0);
        if (send_bytes == -1) {
            perror("Client send error");
            exit(EXIT_FAILURE);
        }
        request += send_bytes;
        request_len -= send_bytes;
    }
}

char * receiveFromServer() {
    sendRequestToServer();
    clearClientData();

    ssize_t recv_bytes = MESSAGE_MAX_SIZE;
    while (recv_bytes == MESSAGE_MAX_SIZE) {
        recv_bytes = recv(FS_CLIENT.sock, FS_CLIENT.response + FS_CLIENT.response_len, MESSAGE_MAX_SIZE, 0);
        if (recv_bytes == 0) {
            return NULL;
        } else if (recv_bytes == -1) {
            perror("Client receive error");
            exit(EXIT_FAILURE);
        }
        FS_CLIENT.response_len += recv_bytes;

        if (FS_CLIENT.response_len + MESSAGE_MAX_SIZE > FS_CLIENT.response_cap) {
            FS_CLIENT.response_cap += MESSAGE_MAX_SIZE;
            FS_CLIENT.response = realloc(FS_CLIENT.response, FS_CLIENT.response_cap);
        }
    }

    return FS_CLIENT.response;
}

void deleteClient() {
    shutdown(FS_CLIENT.sock, SHUT_RDWR);
    close(FS_CLIENT.sock);
}
