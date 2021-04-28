#include "Client.h"

int main() {

    initClient();

    char* server_response;
    while ((server_response = receiveFromServer()) != NULL) {
        printf("%s", server_response);
        fflush(stdout);

        char* user_command = NULL;
        size_t user_command_len = 0;
        user_command_len = getline(&user_command, &user_command_len, stdin);
        sendToServer(user_command);
        free(user_command);
    }

    deleteClient();

    return 0;
}
