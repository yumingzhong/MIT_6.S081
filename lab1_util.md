# Lab util: Unix utilities
## Task 1: sleep
- 目标：暂停用户指定的tick数
- 实现：直接调用系统调用

<details>
<summary> show code </summary>

```C
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
```

</details>

## Task 2: pingpong
- 目标：父子进程通过管道通信

<details>
<summary> show code </summary>

```C
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
```

</details>

## Task 3: primes
- 目标：使用管道编写素数筛的并行版本
- 实现：进程从管道读取数据，第一个数字一定是素数，输出；然后丢弃剩余数据中该素数的倍数；若有数据剩余，创建管道和进程，将剩余数据通过管道传递给下一个进程

<details>
<summary> show code </summary>

```C
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
```

</details>

## Task 4: find
- 目标：在给定目录下查找文件名为给定文件名的所有文件

<details>
<summary> show code </summary>

```C
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define kPathMaxSize 512
char path[kPathMaxSize+1];
char file_name[DIRSIZ+1];
char target[DIRSIZ+1];
struct stat st;
struct dirent dir;

void path2FileName() {
    // get the file name from a given path 
    char *name_start = path + strlen(path);
    while (name_start >= path && *name_start != '/') {
        --name_start;
    }
    ++name_start;

    if (strlen(name_start) > DIRSIZ) {
        fprintf(2, "The path is too long\n");
        exit(1);
    }
    strcpy(file_name, name_start);
}

void find() {
    if (stat(path, &st) < 0) {
        fprintf(2, "cann't stat %s\n", path);
        exit(1);
    }

    int fd = open(path, 0);
    if (fd < 0) {
        fprintf(2, "cann't open %s\n", path);
        exit(1);
    }
    char *p = path + strlen(path);
    switch (st.type) {
        case T_FILE:
            // file: compare file name and target
            path2FileName();
            if (!strcmp(file_name, target))
                printf("%s\n", path);
            break;
        case T_DIR:
            while (read(fd, &dir, sizeof(dir)) == sizeof(dir)) {
                // ignore "." and ".."
                if (!dir.inum || !strcmp(dir.name, ".") || !strcmp(dir.name, ".."))
                    continue;

                if (strlen(path) + 1 + strlen(dir.name) + 1 > sizeof(path)) {
                    fprintf(2, "The path is too long\n");
                    exit(1);
                }

                // dir: recursively search every file in this dir
                p[0] = '/';
                strcpy(p+1, dir.name);
                find();
                p[0] = '\0';
            }
            break;
    }
    close(fd);
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        strcpy(path, ".");
        if (strlen(argv[1]) > DIRSIZ) {
            fprintf(2, "The file name of %s is too long\n", argv[1]);
            exit(1);
        }
        strcpy(target, argv[1]);
    } else if (argc == 3) {
        strcpy(path, argv[1]);
        if (strlen(argv[2]) > DIRSIZ) {
            fprintf(2, "The file name of %s is too long\n", argv[2]);
            exit(1);
        }
        strcpy(target, argv[2]);
    } else {
        fprintf(2, "invalid number of parameters\n");
        exit(1);
    }

    find();
    exit(0);
}
```

</details>

## Task 5: xargs
- 目标：实现简单版本的xargs
- 实现：解析输入，根据\n将输入分为多行；对每行输入参数用空格划分，将输入参数附加到原参数后；对每行参数调用一次exec

<details>
<summary> show code </summary>

```C
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

```

</details>