
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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
#include "proto.h"

// global variables
// char current_dir[64] = "/";


/*****************************************************************************
 *                               kernel_main
 *****************************************************************************/
/**
 * jmp from kernel.asm::_start. 
 * 
 *****************************************************************************/
PUBLIC int kernel_main()
{
	disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
		 "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

	int i, j, eflags, prio;
        u8  rpl;
        u8  priv; /* privilege */

	struct task * t;
	struct proc * p = proc_table;

	char * stk = task_stack + STACK_SIZE_TOTAL;

	for (i = 0; i < NR_TASKS + NR_PROCS; i++,p++,t++) {
		if (i >= NR_TASKS + NR_NATIVE_PROCS) {
			p->p_flags = FREE_SLOT;
			continue;
		}

	        if (i < NR_TASKS) {     /* TASK */
                        t	= task_table + i;
                        priv	= PRIVILEGE_TASK;
                        rpl     = RPL_TASK;
                        eflags  = 0x1202;/* IF=1, IOPL=1, bit 2 is always 1 */
			prio    = 15;
                }
                else {                  /* USER PROC */
                        t	= user_proc_table + (i - NR_TASKS);
                        priv	= PRIVILEGE_USER;
                        rpl     = RPL_USER;
                        eflags  = 0x202;	/* IF=1, bit 2 is always 1 */
			prio    = 5;
                }

		strcpy(p->name, t->name);	/* name of the process */
		p->p_parent = NO_TASK;

		if (strcmp(t->name, "INIT") != 0) {
			p->ldts[INDEX_LDT_C]  = gdt[SELECTOR_KERNEL_CS >> 3];
			p->ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DS >> 3];

			/* change the DPLs */
			p->ldts[INDEX_LDT_C].attr1  = DA_C   | priv << 5;
			p->ldts[INDEX_LDT_RW].attr1 = DA_DRW | priv << 5;
		}
		else {		/* INIT process */
			unsigned int k_base;
			unsigned int k_limit;
			int ret = get_kernel_map(&k_base, &k_limit);
			assert(ret == 0);
			init_desc(&p->ldts[INDEX_LDT_C],
				  0, /* bytes before the entry point
				      * are useless (wasted) for the
				      * INIT process, doesn't matter
				      */
				  (k_base + k_limit) >> LIMIT_4K_SHIFT,
				  DA_32 | DA_LIMIT_4K | DA_C | priv << 5);

			init_desc(&p->ldts[INDEX_LDT_RW],
				  0, /* bytes before the entry point
				      * are useless (wasted) for the
				      * INIT process, doesn't matter
				      */
				  (k_base + k_limit) >> LIMIT_4K_SHIFT,
				  DA_32 | DA_LIMIT_4K | DA_DRW | priv << 5);
		}

		p->regs.cs = INDEX_LDT_C << 3 |	SA_TIL | rpl;
		p->regs.ds =
			p->regs.es =
			p->regs.fs =
			p->regs.ss = INDEX_LDT_RW << 3 | SA_TIL | rpl;
		p->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p->regs.eip	= (u32)t->initial_eip;
		p->regs.esp	= (u32)stk;
		p->regs.eflags	= eflags;

		p->ticks = p->priority = prio;

		p->p_flags = 0;
		p->p_msg = 0;
		p->p_recvfrom = NO_TASK;
		p->p_sendto = NO_TASK;
		p->has_int_msg = 0;
		p->q_sending = 0;
		p->next_sending = 0;

		for (j = 0; j < NR_FILES; j++)
			p->filp[j] = 0;

		stk -= t->stacksize;
	}

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
    init_keyboard();

	restart();

	while(1){}
}


/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}


/**
 * @struct posix_tar_header
 * Borrowed from GNU `tar'
 */
struct posix_tar_header
{				/* byte offset */
	char name[100];		/*   0 */
	char mode[8];		/* 100 */
	char uid[8];		/* 108 */
	char gid[8];		/* 116 */
	char size[12];		/* 124 */
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100];	/* 157 */
	char magic[6];		/* 257 */
	char version[2];	/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];	/* 329 */
	char devminor[8];	/* 337 */
	char prefix[155];	/* 345 */
	/* 500 */
};

/*****************************************************************************
 *                                untar
 *****************************************************************************/
/**
 * Extract the tar file and store them.
 * 
 * @param filename The tar file.
 *****************************************************************************/
void untar(const char * filename)
{
	printf("[extract `%s'\n", filename);
	int fd = open(filename, O_RDWR);
	assert(fd != -1);

	char buf[SECTOR_SIZE * 16];
	int chunk = sizeof(buf);

	while (1) {
		read(fd, buf, SECTOR_SIZE);
		if (buf[0] == 0)
			break;

		struct posix_tar_header * phdr = (struct posix_tar_header *)buf;

		/* calculate the file size */
		char * p = phdr->size;
		int f_len = 0;
		while (*p)
			f_len = (f_len * 8) + (*p++ - '0'); /* octal */

		int bytes_left = f_len;
		char bin[64] = "/bin/";
		if (!strcmp("kernel.bin", phdr->name)) {
			strcpy(bin, phdr->name);
		} else {
			strcat(bin, phdr->name);
		}

		int fdout = open(bin, O_CREAT | O_RDWR);
		if (fdout == -1) {
			printf("    failed to extract file: %s\n", bin);
			printf(" aborted]");
			return;
		}
		printf("    %s (%d bytes)\n", bin, f_len);
		while (bytes_left) {
			int iobytes = min(chunk, bytes_left);
			read(fd, buf,
			     ((iobytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
			write(fdout, buf, iobytes);
			bytes_left -= iobytes;
		}
		close(fdout);
	}

	close(fd);

	printf(" done]\n");
}

void clear() {
	int i;
	for (i=0;i<25;++i) printl("\n");
	// clear_screen(0, console_table[current_console].cursor);
    // console_table[current_console].crtc_start = console_table[current_console].orig;
	// console_table[current_console].cursor = console_table[current_console].orig;
}

void ps() {
	int i;
	printf(" pid |   name   | pri | parent_pid\n");
	for (i = 0; i < NR_TASKS + NR_PROCS; ++i) {
		struct proc* p = &proc_table[i];
		if (p->p_flags != FREE_SLOT) {
			printf("%3d  %7s  %6d  %7d\n", i, p->name, p->priority, p->p_parent == NO_TASK ? 0 : p->p_parent);
		}
	}
}


/**
 * Built-in command: change directory.
 * TODO: 
 * 	1.check length;
 * 	2.more complicated grammar	
 *****************************************************************************/
int cd(char * path){
	if (chdir(path) != 0) {
		printf("cd: No such file or directory!\n");
	}
	// // null check
	// if(strlen(path) == 0){
	// 	return 1;
	// }

	// char next_current_dir[64];
	// for(int i = 0; i < 64; i++){
	// 	next_current_dir[i] = 0;
	// }
	// next_current_dir[0] = '/';

	// // update current folder
	// // slash, absolute path
	// if(path[0] == '/'){
	// 	strcpy(next_current_dir,path);
	// }
	// // dot
	// else if(path[0] == '.'){
	// 	// current folder
	// 	if(strlen(path) > 2 && path[1] == '/'){
	// 		char * p = path;
	// 		p += 2;

	// 		strcpy(next_current_dir, current_dir);
	// 		strcat(next_current_dir, p);
	// 	}
	// 	// parent folder
	// 	else if(path[1] == '.' && (strlen(path) == 2 || path[2] == '/')){
	// 		// find last slash except the one at the last
	// 		int cut_pos = 0;
	// 		strcpy(next_current_dir, current_dir);
	// 		for(int i = strlen(next_current_dir) - 2;i >= 0; i--){
	// 			if(next_current_dir[i] == '/'){
	// 				cut_pos = i;
	// 				break;
	// 			}
	// 		}

	// 		// cut
	// 		for(int i = cut_pos + 1; i < 64; i++){
	// 			next_current_dir[i] = 0;
	// 		}

	// 		// catenate
	// 		if(strlen(path) > 3){
	// 			char * p = path;
	// 			p += 3;
	// 			strcat(next_current_dir,p);
	// 		}
	// 	}
	// }
	// // current folder
	// else{
	// 	strcpy(next_current_dir, current_dir);
	// 	strcat(next_current_dir,path);
	// }

	// if(next_current_dir[strlen(next_current_dir) - 1] != '/'){
	// 	next_current_dir[strlen(next_current_dir)] = '/';
	// }

	// int fd = open(next_current_dir, O_RDWR);

	// if(fd == -1){
	// 	printf("Cannot find directory.\n");
	// }
	// else{
	// 	close(fd);
	// 	strcpy(current_dir, next_current_dir);
	// }
}

/*****************************************************************************
 *                                shabby_shell
 *****************************************************************************/
/**
 * A very very simple shell.
 * 
 * @param tty_name  TTY file name.
 *****************************************************************************/
void shabby_shell(const char * tty_name)
{
	int fd_stdin  = open(tty_name, O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	boot_animation();
	clear();
	boot_banner();

	char rdbuf[128];
	char path[128];

	while (1) {
		getpwd(path, 128);
		// show (username and) current directory
		char message[128] = "";
		strcat(message, "root@NyanCatOS: ");
		strcat(message, path);
		strcat(message, " $ ");
	
		write(1, message, strlen(message));
		int r = read(0, rdbuf, 70);
		rdbuf[r] = 0;

		int argc = 0;
		char * argv[PROC_ORIGIN_STACK];
		char * p = rdbuf;
		char * s;
		int word = 0;
		char ch;
		do {
			ch = *p;
			if (*p != ' ' && *p != 0 && !word) {
				s = p;
				word = 1;
			}
			if ((*p == ' ' || *p == 0) && word) {
				word = 0;
				argv[argc++] = s;
				*p = 0;
			}
			p++;
		} while(ch);
		argv[argc] = 0;

		if (buildin_command(argv)) {
			continue;
		}

		char bin[128] = "/bin/";
		strcat(bin, argv[0]);
		int fd = open(bin, O_RDWR);
		if (fd == -1) {
			printl("Command not found!\n");
			// if (rdbuf[0]) {
			// 	write(1, "{", 1);
			// 	write(1, rdbuf, r);
			// 	write(1, "}\n", 2);
			// }
		}
		else {
			close(fd);
			int pid = fork();
			if (pid != 0) { /* parent */
				int s;
				wait(&s);
			}
			else {	/* child */
				execv(bin, argv);
			}
		}
	}

	close(1);
	close(0);
}

int buildin_command(char **argv) {
	if (!strcmp(argv[0], "clear")) {
		clear();
		return 1;
	} else if (!strcmp(argv[0], "ps")) {
		ps();
		return 1;
	} else if(!strcmp(argv[0], "cd")) {
		cd(argv[1]);
		return 1;
	}
	return 0;
}

/*****************************************************************************
 *                                Init
 *****************************************************************************/
/**
 * The hen.
 * 
 *****************************************************************************/
void Init()
{
	int fd_stdin  = open("/dev_tty0", O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open("/dev_tty0", O_RDWR);
	assert(fd_stdout == 1);

	mkdir("/bin");

	/* extract `cmd.tar' */
	untar("/cmd.tar");
			

	char * tty_list[] = {"/dev_tty0"};

	int i;
	for (i = 0; i < sizeof(tty_list) / sizeof(tty_list[0]); i++) {
		int pid = fork();
		if (pid != 0) { /* parent process */
		}
		else {	/* child process */
			close(fd_stdin);
			close(fd_stdout);
			
			shabby_shell(tty_list[i]);
			assert(0);
		}
	}

	while (1) {
		int s;
		int child = wait(&s);
		printf("child (%d) exited with status: %d.\n", child, s);
	}

	assert(0);
}


/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	for(;;);
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	for(;;);
}

/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()
{
	for(;;);
}

/*****************************************************************************
 *                                panic
 *****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}