#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define kLineMaxSize 512
char *arguments[MAXARG+1];
char line_argument[kLineMaxSize+1];

int getLineArg(int argc) {
    // read a line
    int n = 0;
    while (read(0, line_argument+n, sizeof(char))) {
        if (n == kLineMaxSize) {
            fprintf(2, "The argument is too long\n");
            exit(1);
        }
        if (line_argument[n] == '\n')
            break;

        ++n;
    }
    line_argument[n] = '\0';

    // use ' ' to split
    char *p = line_argument;
    while (*p) {
        if (argc == MAXARG) {
            fprintf(2, "too many arguments\n");
            exit(1);
        }
        arguments[argc++] = p;

        while (*p && (*p != ' ' || *p != '\n'))
            ++p;
        while (*p && (*p == ' ' || *p == '\n')) 
            *p++ = '\0';
    }
    arguments[argc] = 0;

    return n;
}

int main(int argc, char *argv[]) {
    int i;
    for (i = 0; i < argc-1; ++i) 
        arguments[i] = argv[i+1];

    while (getLineArg(argc-1)) {
        if (!fork()) {
            exec(arguments[0], arguments);
            fprintf(2, "exec error\n");
            exit(1);
        } else {
            wait(0);
        }
        arguments[argc-1] = 0;
    }

    exit(0);
}
