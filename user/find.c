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
