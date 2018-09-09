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

PUBLIC int fstat(int fd, const char *filename, struct stat *buf) {
    MESSAGE msg;

	msg.type	= FSTAT;

	msg.FD	    = fd;
    msg.PATHNAME    = (void*)filename;
	msg.BUF		= (void*)buf;
    msg.NAME_LEN	= strlen(filename);

	send_recv(BOTH, TASK_FS, &msg);
	assert(msg.type == SYSCALL_RET);

	return msg.RETVAL;
}