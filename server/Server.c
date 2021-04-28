#include "Server.h"

#define MESSAGE_MAX_SIZE 4096

void demonize() {
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);
    chdir("/");
    openlog("mydaemon",
            LOG_PID,
            LOG_DAEMON);
}

void initServer() {
    //demonize();

    FS_SERVER.listener = socket(AF_INET, SOCK_STREAM, 0);
    FS_SERVER.addr.sin_family = AF_INET;
    FS_SERVER.addr.sin_port = htons(3425);
    FS_SERVER.addr.sin_addr.s_addr = htonl(INADDR_ANY);

    FS_SERVER.request_cap = 2 * MESSAGE_MAX_SIZE;
    FS_SERVER.response_cap = 2 * MESSAGE_MAX_SIZE;
    FS_SERVER.request = calloc(FS_SERVER.request_cap, 1);
    FS_SERVER.response = calloc(FS_SERVER.response_cap, 1);
    FS_SERVER.request_len = 0;
    FS_SERVER.response_len = 0;

    int reuse_val = 1;
    setsockopt(FS_SERVER.listener, SOL_SOCKET, SO_REUSEADDR, &reuse_val, sizeof(reuse_val));
    setsockopt(FS_SERVER.listener, SOL_SOCKET, SO_REUSEPORT, &reuse_val, sizeof(reuse_val));

    if (bind(FS_SERVER.listener, (struct sockaddr *) &FS_SERVER.addr, sizeof(FS_SERVER.addr)) == -1) {
        perror("Server bind error");
        exit(EXIT_FAILURE);
    }
}

void acceptClient() {
    listen(FS_SERVER.listener, 1);
    FS_SERVER.sock = accept(FS_SERVER.listener, NULL, NULL);
}

void clearServerData() {
    if (FS_SERVER.request_cap > 2 * MESSAGE_MAX_SIZE) {
        FS_SERVER.request = realloc(FS_SERVER.request, 2 * MESSAGE_MAX_SIZE);
    }
    if (FS_SERVER.response_cap > 2 * MESSAGE_MAX_SIZE) {
        FS_SERVER.response = realloc(FS_SERVER.response, 2 * MESSAGE_MAX_SIZE);
    }

    FS_SERVER.request_cap = 2 * MESSAGE_MAX_SIZE;
    FS_SERVER.response_cap = 2 * MESSAGE_MAX_SIZE;

    FS_SERVER.request_len = 0;
    FS_SERVER.response_len = 0;

    memset(FS_SERVER.request, 0, FS_SERVER.request_cap);
    memset(FS_SERVER.response, 0, FS_SERVER.response_cap);
}

// This is an "abstraction" function
// It construct FS_SERVER.response
void sendToClient(char* new_response) {
    uint32_t response_len = strlen(new_response);
    if (FS_SERVER.response_len + response_len >= FS_SERVER.response_cap) {
        FS_SERVER.response_cap = FS_SERVER.response_len + response_len + 1;
        FS_SERVER.response = realloc(FS_SERVER.response, FS_SERVER.response_cap);
    }
    memcpy(FS_SERVER.response + FS_SERVER.response_len, new_response, response_len);
    FS_SERVER.response_len += response_len;
}

void sendResponseToClient() {
    char* response = FS_SERVER.response;
    uint32_t response_len = FS_SERVER.response_len;

    while (response_len != 0) {

        size_t message_len = response_len;
        if (message_len > MESSAGE_MAX_SIZE) {
            message_len = MESSAGE_MAX_SIZE;
        }

        ssize_t send_bytes = send(FS_SERVER.sock, response, message_len, 0);
        if (send_bytes == -1) {
            perror("Server send error");
            exit(EXIT_FAILURE);
        }
        response += send_bytes;
        response_len -= send_bytes;
    }
}

char* receiveFromClient() {
    sendResponseToClient();
    clearServerData();

    puts(FS_SERVER.request);

    ssize_t recv_bytes = MESSAGE_MAX_SIZE;
    while (recv_bytes == MESSAGE_MAX_SIZE) {
        recv_bytes = recv(FS_SERVER.sock, FS_SERVER.request + FS_SERVER.request_len, MESSAGE_MAX_SIZE, 0);
        if (recv_bytes == -1) {
            perror("Server recv error");
            exit(EXIT_FAILURE);
        }
        FS_SERVER.request_len += recv_bytes;

        if (FS_SERVER.request_len + MESSAGE_MAX_SIZE > FS_SERVER.request_cap) {
            FS_SERVER.request_cap += 2 * MESSAGE_MAX_SIZE;
            FS_SERVER.request = realloc(FS_SERVER.request, FS_SERVER.request_cap);
        }
    }

    return FS_SERVER.request;
}

void destructServer() {
    shutdown(FS_SERVER.sock, SHUT_RDWR);
    close(FS_SERVER.sock);

    shutdown(FS_SERVER.listener, SHUT_RDWR);
    close(FS_SERVER.listener);

    free(FS_SERVER.request);
    free(FS_SERVER.response);
}
