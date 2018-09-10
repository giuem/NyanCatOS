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

PUBLIC int chdir(const char * path) {
    MESSAGE msg;

    msg.type = CHDIR;

    msg.PATHNAME = (void *)path;
    msg.NAME_LEN = strlen(path);

    send_recv(BOTH, TASK_FS, &msg);
	assert(msg.type == SYSCALL_RET);

	return msg.RETVAL;
}