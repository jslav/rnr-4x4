/*
 * Copyright (c) 2020 Yuri Bobrov <ubobrov@yandex.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed as is in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <byteswap.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sched.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/syscall.h>
#include <linux/videodev2.h>

#include "video_device.h"

/*
 *
 */
void errno_exit(const char *s) {
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(-1);
}

/*
 *
 */
int xioctl(int fh, int request, void *arg) {
    int r;

    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

/*
 *
 */
int dev_try_format(int fd, int w, int h, int fmtid) {
    struct v4l2_format fmt;

    fmt.fmt.pix.width       = w;
    fmt.fmt.pix.height      = h;
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = fmtid;
    fmt.fmt.pix.field       = V4L2_FIELD_ANY;

    if (xioctl(fd, VIDIOC_TRY_FMT, &fmt) < 0) {
       return -1;
    }

    if(fmt.fmt.pix.pixelformat != fmtid) {
        return -1;
    }

    if (xioctl(fd, VIDIOC_S_FMT, &fmt)<0) {
        return -1;
    }
    return 0;
}

/*
 *
 */
int open_capture_dev(char *name, int *fd) { 
    int i;
    v4l2_std_id std_id;
    struct v4l2_capability cap;
    /* open device NONEBLOCK mode to avoid -EAGAIN and CPU load */
    *fd = open(name, O_RDWR /*| O_NONBLOCK*/, 0);
    if (-1 == *fd) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", name, errno, strerror(errno));
        return -1;
    }

    /* get standard (wait for it to be locked onto a signal) */
    if (-1 == xioctl(*fd, VIDIOC_G_STD, &std_id))
        perror("VIDIOC_G_STD");

    for (i = 0; std_id == V4L2_STD_ALL && i < 10; i++) {
        usleep(100000);
        xioctl(*fd, VIDIOC_G_STD, &std_id);
    }
    /* set the standard to the detected standard (this is critical for autodetect) */
    if (std_id != V4L2_STD_UNKNOWN) {
        if (-1 == xioctl(*fd, VIDIOC_S_STD, &std_id))
            perror("VIDIOC_S_STD");
        if (std_id & V4L2_STD_NTSC)
            printf("found NTSC TV decoder\n");
        if (std_id & V4L2_STD_SECAM)
            printf("found SECAM TV decoder\n");
        if (std_id & V4L2_STD_PAL)
            printf("found PAL TV decoder\n");
    }

    /* ensure device has video capture capability */
    if (-1 == xioctl(*fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s is no V4L2 device\n",name);
            return -1;
        } else {
            return -1;
        }
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "%s is no video capture device\n", name);
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "%s does not support streaming i/o\n", name);
        return -1;
    }
    return 0;
}


