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

PUBLIC int getdents(int fd) {
    MESSAGE msg;
    msg.type = GET_DENTS;
    msg.FD = fd;

    send_recv(BOTH, TASK_FS, &msg);
    return msg.RETVAL;
}