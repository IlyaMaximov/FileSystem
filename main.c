#include <stdlib.h>
#include "console/Console.h"

int main() {

    MiniFs* miniFs = calloc(sizeof(MiniFs), 1);
    initMiniFs(miniFs);

    runFs(miniFs);

    free(miniFs);
    return 0;
}
