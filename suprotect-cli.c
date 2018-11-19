#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>

#include "suprotect.h"

#define DEVICE_FILE_NAME "/dev/" SUPROTECT_DEV_NAME

int main(int argc, char *argv[])
{
    int ret = 0;
    struct suprotect_request params;
    int fd;

    if (argc != 5) {
        printf("Usage: %s <pid> <addr> <len> <prot>\n", argv[0]);
        printf("Note: all the parameters are in hex, except the pid.\n");
        return -1;
    }

    fd = open(DEVICE_FILE_NAME, 0);
    if (fd < 0) {
        fprintf(stderr, "Could not open device file: %s\n", DEVICE_FILE_NAME);
        return -1;
    }

    params.pid = atoi(argv[1]);
    params.addr = (void *)strtol(argv[2], NULL, 16);
    params.len = strtol(argv[3], NULL, 16);
    params.prot = strtol(argv[4], NULL, 16);

    ret = ioctl(fd, SUPROTECT_IOCTL_MPROTECT, &params);
    if (ret < 0)
        perror("IOCTL error");

    close(fd);

    return ret;
}

