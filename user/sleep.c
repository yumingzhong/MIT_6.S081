#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(2, "The number of parameters doesn't meet the requirements.\n");
        exit(1);
    }

    uint sleep_ticks = atoi(argv[1]);
    // invoke system call
    sleep(sleep_ticks);

    exit(0);
}