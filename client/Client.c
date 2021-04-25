#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <libsync.h>

#define MESSAGE_SIZE 4096

int main() {

    char request[MESSAGE_SIZE] = "I ready";
    char response[MESSAGE_SIZE];

    int sock;
    struct sockaddr_in addr;
    size_t read_bytes = 0;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Connect error");
        exit(EXIT_FAILURE);
    }

    while (true) {
        // size fix
        send(sock, request, strlen(request), 0);

        do {
            read_bytes = recv(sock, response, sizeof(response), 0);
            printf("%s\n", response);
            fflush(stdout);
            memset(response, 0, MESSAGE_SIZE);

        } while (read_bytes == MESSAGE_SIZE);

        memset(request, 0, MESSAGE_SIZE);
        scanf("%s", request);
    }

//    free(miniFs);
    return 0;
}
