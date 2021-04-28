#include <stdlib.h>
#include "Server.h"
#include "../console/Console.h"

#define MESSAGE_SIZE 4096


int main() {

    initServer();
    acceptClient();

    MiniFs* miniFs = calloc(sizeof(MiniFs), 1);
    initMiniFs(miniFs);
    runFs(miniFs);
    free(miniFs);

    destructServer();

    return 0;
}
