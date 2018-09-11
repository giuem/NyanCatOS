#include "stdio.h"
#include "sys/const.h"
#include "sys/fs.h"
#include "string.h"

#define F_SHOW_ALL 0x01
#define F_SHOW_DETAIL 0x02
#define F_SHOW_HELP 0x04

void help() {
    printf("Usage: ls [-a] [-l] [-h] [DIR]\n\n");
    printf(" -a   do not ignore entries starting with .\n");
    printf(" -l   use a long listing format\n");
    printf(" -h   show help\n");
}

void do_ls(const char* path) {
    
}

int main(int argc, char const *argv[])
{
    int flag = 0;

    char *filename = ".";

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (strchr(argv[i], 'l')) {
                flag |= F_SHOW_ALL;
            }
            if (strchr(argv[i], 'a')) {
                flag |= F_SHOW_DETAIL;
            }
            if (strchr(argv[i], 'h')) {
                flag |= F_SHOW_HELP;
                help();
                return 0;
            }
        }
    }

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            filename = argv[i];
            break;
        }
    }

    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        printf("fail to ls %s\n", filename);
        return -1;
    }
    struct stat st;
    struct dir_entry dirent;
    int i;

    if (stat(filename, &st) < 0)
    {
        printf("ls: cannot stat %s\n", filename);
        close(fd);
        return -1;
    }

    const int cnt = st.st_size / DIR_ENTRY_SIZE;

    switch (st.st_mode)
    {
    case I_REGULAR:
        break;
    case I_DIRECTORY:
        for (i = 0; i < cnt; ++i) {
            read(fd, &dirent, sizeof(dirent));
            if (!(flag & F_SHOW_ALL) && dirent.name[0] == '.') {
                // hidden file
                continue;
            }
            // printf("%2d %s \n", dirent.inode_nr, dirent.name);
            if (fstat(fd, dirent.name, &st) < 0) {
                // printf("cannot stat %s\n", dirent.name);
                continue;
            }
            if (dirent.inode_nr > 0) {
                if (flag & F_SHOW_DETAIL) {
                    printf("%3d %7d %s%s\n", dirent.inode_nr, st.st_size,
                        dirent.name, st.st_mode==I_DIRECTORY ? "/" : "");
                } else {
                    printf("%s  ", dirent.name);
                }
            }
        }
        if (!(flag & F_SHOW_DETAIL)) printf("\n");
        break;
    default:
        printf("ls: cannot ls unsupported file %s\n", filename);
        break;
    }
    close(fd);
    return 0;
}


