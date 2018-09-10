#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE int addpath (const char *path, char **ap, const char *entry)
{
	const char *e= entry;
	char *p= *ap;

	while (*e != 0) e++;

	while (e > entry && p > path) *--p = *--e;

	if (p == path) return -1;
	*--p = '/';
	*ap= p;
	return 0;
}

PRIVATE int recover ( char * p )
{
	int slash;
	char *p0;

	while (*p != 0) {
		p0= ++p;

		do p++; while (*p != 0 && *p != '/');
		slash= *p; *p= 0;

		if (chdir(p0) != 0) return -1;
		*p= slash;
	}
	return 0;
} 

PUBLIC int getpwd(char *path, int size)
{
	struct stat above, current;
	struct dir_entry dirent;
	char *p, *up;
    const char dot[] = ".";
    const char dotdot[] = "..";
	int cycle;

	if (0 == path || size <= 1) {
		printl("%s %d null path pointer\n");
		return -1;
	}

	p = path + size;
	*--p = '\0';
	if (stat(dot, &current) < 0) {
		printl("%s %d stat current failed\n");
		return -1;
	}
    int fd = -1;
    int i;
    int dir_size;

	while (1) {
		if (stat(dotdot, &above) < 0) {

			printl("%s %d stat dotdot failed\n", __FILE__, __LINE__);
			recover(p);
			return -1;
		}

		if (above.st_dev == current.st_dev
				&& above.st_ino == current.st_ino) {
			//found the ROOT_INODE
			// printl("%s %d found the root inode : d_ino %d\n", __FILE__, __LINE__, current.st_ino);
			break;
		}

        fd = open(dotdot, O_RDWR);
        if (fd == -1) {
            recover(p);
			return -1;
        }

        dir_size = above.st_size/DIR_ENTRY_SIZE;
        for (i = 0; i < dir_size; ++i) {
            read(fd, &dirent, sizeof(dirent));
            if (strcmp(dirent.name, ".") == 0) continue;
            if (strcmp(dirent.name, "..") == 0) continue;

            if (dirent.inode_nr == current.st_ino) {
                break;
            }
        }

        close(fd);

        up = p;
        if (addpath(path, &up, dirent.name) < 0) {
			printl("%s %d add path failed\n", __FILE__, __LINE__);
			recover(p);
			return -1;
		}

        if (chdir(dotdot) != 0) {
            printl("%s %d chdir dotdot failed\n", __FILE__, __LINE__);
			recover(p);
			return -1;
        }

        p = up;

        current = above;

		// if (NULL == (d = opendir(dotdot)) ) {
		// 	// printl("%s %d open dotdot failed\n", __FILE__, __LINE__);
		// 	recover(p);
		// 	return NULL;
		// }

		/*  Cycle is 0 for a simple inode nr search, or 1 for a search
		 *  for inode *and* device nr.
		*/
		// cycle= above.st_dev == current.st_dev ? 0 : 1;

		// do {
		// 	char name[3 + MAX_FILENAME_LEN + 1];

		// 	tmp.st_ino = 0;
		// 	if (NULL == (entry = readdir(d))) {
		// 		switch (++cycle) {
		// 			case 1:
		// 				rewinddir(d);
		// 				continue;
		// 			case 2:
		// 				printl("%s %d read dir failed\n", __FILE__, __LINE__);
		// 				closedir(d);
		// 				recover(p);
		// 				return NULL;
		// 		}
		// 	}

		// 	if (0 == strcmp(entry->d_name, ".")) continue;
		// 	if (0 == strcmp(entry->d_name, "..")) continue;

		// 	switch (cycle) {
		// 		case 0:
		// 			if (entry->d_ino != current.st_ino) continue;
		// 		case 1:
		// 			strcpy(name, "../");
		// 			strcpy(name + 3, entry->d_name);
		// 			if (stat(name, &tmp) < 0)
		// 				continue;
		// 			break;
		// 	}
		// } while (tmp.st_ino != current.st_ino
		// 		|| tmp.st_dev != current.st_dev);

		// up = p;
		// if (addpath(path, &up, entry->d_name) < 0) {
		// 	printl("%s %d add path failed\n", __FILE__, __LINE__);
		// 	closedir(d);
		// 	recover(p);
		// 	return NULL;
		// }
		// closedir(d);

		// if (chdir(dotdot) != 0) {
		// 	printl("%s %d chdir dotdot failed\n", __FILE__, __LINE__);
		// 	recover(p);
		// 	return NULL;
		// }

		// p = up;

		// current = above;
	}
	if (recover(p) != 0) {
		printl("%s %d fail to undo all the chdir dotdot\n", __FILE__, __LINE__);
		return -1;
	}
	if (*p == 0) {
		*--p = '/';
	}
	if (p > path) {
		//move string to the start of path
		strcpy(path, p);
	}
    // printl("%s\n", path);
	return 0;
}