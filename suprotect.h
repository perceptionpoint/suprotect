#ifndef _SUPROTECT_H
#define _SUPROTECT_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define SUPROTECT_DEV_NAME          "suprotect"
#define SUPROTECT_IOCTL_MPROTECT    _IOW('7', 1, struct suprotect_request)

struct suprotect_request {
    /* pid of the target task */
    pid_t pid;

    /* parameters to mprotect */
    void *addr;
    size_t len;
    int prot;
};

#endif /* _SUPROTECT_H */

