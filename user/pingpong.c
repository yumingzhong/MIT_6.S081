#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define kMessageLen 5

int main() {
    int parent2child[2], child2parent[2];
    pipe(parent2child);
    pipe(child2parent);
   
    char message[kMessageLen] = "ping";
    write(parent2child[1], message, kMessageLen);

    if (fork()) {
        // parent
        wait(0);

        close(parent2child[0]);
        close(child2parent[1]);

        read(child2parent[0], message, kMessageLen);
        int pid = getpid();
        printf("%d: received %s\n", pid, message);

        close(parent2child[1]);
        close(child2parent[0]);
    } else {
        // child
        close(parent2child[1]);
        close(child2parent[0]);

        read(parent2child[0], message, kMessageLen);
        int pid = getpid();
        printf("%d: received %s\n", pid, message);

        strcpy(message, "pong");
        write(child2parent[1], message, kMessageLen);

        close(parent2child[0]);
        close(child2parent[1]);
    }

    exit(0);
}
