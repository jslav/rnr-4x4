#ifndef VIDEO_DEVICE_H
#define VIDEO_DEVICE_H



#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define V4L2MMAP_NBBUFFER 4
#define FOURCC_PRINTF_PARMS(x) (x), (x) >> 8, (x) >> 16, (x) >> 24

int open_capture_dev(char *name, int *fd);
int xioctl(int fh, int request, void *arg);
void errno_exit(const char *s);
int dev_try_format(int fd, int w, int h, int fmtid);

#endif
