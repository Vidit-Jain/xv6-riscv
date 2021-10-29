#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int
main(int argc, char* argv[]) {

    int mask = atoi(argv[1]);
    // Set trace with the given mask for the current process
    trace(mask);
    // Execute the command to be straced
    exec(argv[2], &argv[2]);
    exit(0);
}