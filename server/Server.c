#include <stdlib.h>
#include <signal.h>
#include <sys/syslog.h>
#include <netinet/in.h>
#include "../console/Console.h"

#define MESSAGE_SIZE 4096

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

int main() {

//    MiniFs* miniFs = calloc(sizeof(MiniFs), 1);
//    initMiniFs(miniFs);
//    runFs(miniFs);
//    demonize();

    char request[4096];
    char response[4096];

    int sock, listener;
    size_t bytes_read;
    struct sockaddr_in addr;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Bind error");
        exit(EXIT_FAILURE);
    }

    listen(listener, 1);
    sock = accept(listener, NULL, NULL);
    while(true) {
        while(true) {
            // the command to the console should not be long
            bytes_read = recv(sock, request, MESSAGE_SIZE, 0);
            // there should be a console response here
            send(sock, request, bytes_read, 0);
        }
        printf("End command\n");
    }

    close(sock);
    closelog();
//    free(miniFs);
    return 0;
}
