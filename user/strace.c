#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int
main(int argc, char* argv[]) {
    int mask = atoi(argv[1]);
    trace(mask);
    exec(argv[2], &argv[2]);
    exit(0);
}