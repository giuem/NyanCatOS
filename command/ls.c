#include "stdio.h"
#include "sys/const.h"
#include "sys/fs.h"

int main(int argc, char const *argv[])
{
    const char *filename = ".";
    int fd = open(filename, O_RDWR);
    struct stat st;
    struct dir_entry dirent;
    int i;

    if (stat(".", &st) < 0)
    {
        printf("cannot stat %s\n", filename);
    }

    const int cnt = st.st_size / DIR_ENTRY_SIZE;

    switch (st.st_mode)
    {
    case I_REGULAR:
        break;
    case I_DIRECTORY:
        for (i = 0; i < cnt; ++i) {
            read(fd, &dirent, sizeof(dirent));
            // printf("%2d %s \n", dirent.inode_nr, dirent.name);
            if (stat(dirent.name, &st) < 0) {
                printf("cannot stat %s\n", dirent.name);
            }
            printf("%3d %7d %s%s\n", dirent.inode_nr, st.st_size,
                    dirent.name, st.st_mode==I_DIRECTORY ? "/" : "");
        }
        break;
    default:
        printf("cannot ls unsupported file");
        break;
    }
    close(fd);
    return 0;
}
