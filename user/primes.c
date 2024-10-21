#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

const int begin = 2, end = 36;
const int size = sizeof(int);

void primeSieve(int p[], int *pnum) {
    read(p[0], pnum, size);
    int current_prime = *pnum;
    // get prime number
    printf("prime %d\n", current_prime);

    int new_pipe[2], cnt = 0;
    pipe(new_pipe);
    // discard the composite numbers read from the previous pipe
    while (read(p[0], pnum, size)) {
        if (*pnum % current_prime) {
            write(new_pipe[1], pnum, size);
            ++cnt;
        }
    }
    close(p[0]);
    close(new_pipe[1]);

    // fork() and continue work
    if (cnt) {
        if (fork())
            wait(0);
        else
            primeSieve(new_pipe, pnum);
    } else {
        close(new_pipe[0]);
    }
}

int main() {
    int p[2];
    pipe(p);
    int *pnum = malloc(size);
    for (*pnum = begin; *pnum != end; ++*pnum) {
        write(p[1], pnum, size);
    }
    close(p[1]);

    if (fork())
        wait(0);
    else 
        primeSieve(p, pnum);

    free(pnum);
    exit(0);
}
