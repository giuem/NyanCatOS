/*************************************************************************//**
 *****************************************************************************
 * @file   misc.c
 * @brief  
 * @author Forrest Y. Yu
 * @date   2008
 *****************************************************************************
 *****************************************************************************/

/* Orange'S FS */

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
#include "hd.h"
#include "fs.h"

/*****************************************************************************
 *                                do_stat
 *************************************************************************//**
 * Perform the stat() syscall.
 * 
 * @return  On success, zero is returned. On error, -1 is returned.
 *****************************************************************************/
PUBLIC int do_stat()
{
	char pathname[MAX_PATH]; /* parameter from the caller */
	char filename[MAX_PATH]; /* directory has been stipped */

	/* get parameters from the message */
	int name_len = fs_msg.NAME_LEN;	/* length of filename */
	int src = fs_msg.source;	/* caller proc nr. */
	assert(name_len < MAX_PATH);
	phys_copy((void*)va2la(TASK_FS, pathname),    /* to   */
		  (void*)va2la(src, fs_msg.PATHNAME), /* from */
		  name_len);
	pathname[name_len] = 0;	/* terminate the string */

	int inode_nr = search_file(pathname);
	if (inode_nr == INVALID_INODE) {	/* file not found */
		printl("{FS} FS::do_stat():: search_file() returns "
		       "invalid inode: %s\n", pathname);
		return -1;
	}

	struct inode * pin = 0;

	struct inode * dir_inode;
	if (strip_path(filename, pathname, &dir_inode) != 0) {
		/* theoretically never fail here
		 * (it would have failed earlier when
		 *  search_file() was called)
		 */
		assert(0);
	}
	pin = get_inode(dir_inode->i_dev, inode_nr);

	struct stat s;		/* the thing requested */
	s.st_dev = pin->i_dev;
	s.st_ino = pin->i_num;
	s.st_mode= pin->i_mode;
	s.st_rdev= is_special(pin->i_mode) ? pin->i_start_sect : NO_DEV;
	s.st_size= pin->i_size;

	put_inode(pin);

	phys_copy((void*)va2la(src, fs_msg.BUF), /* to   */
		  (void*)va2la(TASK_FS, &s),	 /* from */
		  sizeof(struct stat));

	return 0;
}

PUBLIC int do_fstat()
{
	char filename[MAX_PATH];

	int name_len = fs_msg.NAME_LEN;
	assert(name_len < MAX_PATH);
	int fd = fs_msg.FD;	/* length of filename */
	int src = fs_msg.source;	/* caller proc nr. */

	phys_copy((void*)va2la(TASK_FS, filename),    /* to   */
		  (void*)va2la(src, fs_msg.PATHNAME), /* from */
		  name_len);
	filename[name_len] = 0;	

	assert((pcaller->filp[fd] >= &f_desc_table[0]) &&
	       (pcaller->filp[fd] < &f_desc_table[NR_FILE_DESC]));

	struct inode * pin = pcaller->filp[fd]->fd_inode;

	int inode_nr = search_inode(pin, filename);

	if (inode_nr == 0) {
		return -1; // not found
	}
	
	pin = get_inode(pin->i_dev, inode_nr);

	struct stat s;		/* the thing requested */
	s.st_dev = pin->i_dev;
	s.st_ino = pin->i_num;
	s.st_mode= pin->i_mode;
	s.st_rdev= is_special(pin->i_mode) ? pin->i_start_sect : NO_DEV;
	s.st_size= pin->i_size;

	put_inode(pin);

	phys_copy((void*)va2la(src, fs_msg.BUF), /* to   */
		  (void*)va2la(TASK_FS, &s),	 /* from */
		  sizeof(struct stat));

	return 0;
}

PUBLIC int search_inode(const struct inode * parent, const char * filename) {
	int dir_blk0_nr = parent->i_start_sect;
	int nr_dir_blks = (parent->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	int nr_dir_entries = parent->i_size / DIR_ENTRY_SIZE;
	int i, j;
	int m = 0;
	struct dir_entry * pde;
	for (i = 0; i<nr_dir_blks; i++) {
		RD_SECT(parent->i_dev, dir_blk0_nr + i);
		pde = (struct dir_entry *)fsbuf;
		for (j = 0; j < nr_dir_entries; j++,pde++) {
			if (strcmp(filename, pde->name) == 0)
				return pde->inode_nr;
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			break;
	}

	return 0; // not found
}

/*****************************************************************************
 *                                search_file
 *****************************************************************************/
/**
 * Search the file and return the inode_nr.
 *
 * @param[in] path The full path of the file to search.
 * @return         Ptr to the i-node of the file if successful, otherwise zero.
 * 
 * @see open()
 * @see do_open()
 *****************************************************************************/
PUBLIC int search_file(char * path)
{
	int i, j;

	char filename[MAX_PATH];
	memset(filename, 0, MAX_FILENAME_LEN);
	struct inode * dir_inode;
	if (strip_path(filename, path, &dir_inode) != 0)
		return 0;
	printl("search_file %s %d\n", filename, dir_inode->i_num);
	if (filename[0] == 0)	/* path: "/" */
		return dir_inode->i_num;
	// printl("filename: %s, inode: %d\n", filename, dir_inode->i_num);
	/**
	 * Search the dir for the file.
	 */
	// int dir_blk0_nr = dir_inode->i_start_sect;
	// int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	// int nr_dir_entries =
	//   dir_inode->i_size / DIR_ENTRY_SIZE; /**
	// 				       * including unused slots
	// 				       * (the file has been deleted
	// 				       * but the slot is still there)
	// 				       */
	// int m = 0;
	// struct dir_entry * pde;
	// for (i = 0; i < nr_dir_blks; i++) {
	// 	// printl("1");
	// 	RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
	// 	pde = (struct dir_entry *)fsbuf;
	// 	// printl("2");
	// 	for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
	// 		// printl("3");
	// 		// printl("%s", pde->name);
	// 		if (memcmp(filename, pde->name, MAX_FILENAME_LEN) == 0)
	// 			return pde->inode_nr;
	// 		if (++m > nr_dir_entries)
	// 			break;
	// 	}
	// 	if (m > nr_dir_entries) /* all entries have been iterated */
	// 		break;
	// }

	return search_inode(dir_inode, filename);

	/* file not found */
	return 0;
}

/*****************************************************************************
 *                                strip_path
 *****************************************************************************/
/**
 * Get the basename from the fullpath.
 *
 * In Orange'S FS v1.0, all files are stored in the root directory.
 * There is no sub-folder thing.
 *
 * This routine should be called at the very beginning of file operations
 * such as open(), read() and write(). It accepts the full path and returns
 * two things: the basename and a ptr of the root dir's i-node.
 *
 * e.g. After stip_path(filename, "/blah", ppinode) finishes, we get:
 *      - filename: "blah"
 *      - *ppinode: root_inode
 *      - ret val:  0 (successful)
 *
 * Currently an acceptable pathname should begin with at most one `/'
 * preceding a filename.
 *
 * Filenames may contain any character except '/' and '\\0'.
 *
 * @param[out] filename The string for the result.
 * @param[in]  pathname The full pathname.
 * @param[out] ppinode  The ptr of the dir's inode will be stored here.
 * 
 * @return Zero if success, otherwise the pathname is not valid.
 *****************************************************************************/
PUBLIC int strip_path(char * filename, const char * pathname,
		      struct inode** ppinode)
{
	const char * s = pathname;
	char * t = filename;

	if (s == 0)
		return -1;

	if (*s == '/')
		s++;

	struct inode *pinode_now = root_inode, *ptemp;
	struct dir_entry * pde;
	int dir_blk0_nr, nr_dir_blks, nr_dir_entries, m;
	int i, j;

	while (*s) {		/* check each character */
		if (*s == '/') {
			int flag = 0;
			dir_blk0_nr = pinode_now->i_start_sect;
            nr_dir_blks = (pinode_now->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
            nr_dir_entries = pinode_now->i_size / DIR_ENTRY_SIZE;
			m = 0;
            pde = 0;
            *t = 0;
            for (i = 0; i < nr_dir_blks && flag==0; i++) {
                RD_SECT(pinode_now->i_dev, dir_blk0_nr + i);
                pde = (struct dir_entry *)fsbuf;
                for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
                    //printl("pde->name:%s\n", pde->name);
                    if (strcmp(filename, pde->name) == 0){
                        ptemp = get_inode(pinode_now->i_dev, pde->inode_nr);
                        //printl("out\n");
                        if(ptemp->i_mode == I_DIRECTORY){
                           // printl("in\n");
                            pinode_now = ptemp;
                            flag = 1;
                            break;
                        }
                        //return pde->inode_nr;
                    }
                    if (++m > nr_dir_entries)
                        return -1;
                }
                if (m > nr_dir_entries || flag==0) {
                   // printl("flag==0\n");
                    return -1;
                }
            }
            if(flag == 0){
                return -1;
            }
            t = filename;

            s++;
		} else {
            *t++ = *s++;
            if (t - filename >= MAX_FILENAME_LEN)
                break;
        }
		/* if filename is too long, just truncate it */
		if (t - filename >= MAX_FILENAME_LEN)
			break;
	}
	*t = 0;

	*ppinode = pinode_now;

	return 0;
}

