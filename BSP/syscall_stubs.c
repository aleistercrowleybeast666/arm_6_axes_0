/* There is no host transport in Prototype 0. Newlib I/O fails immediately. */
#include <errno.h>
#include <sys/stat.h>
int _close(int fd)
{
    (void)fd;
    errno = EBADF;
    return -1;
}
int _lseek(int fd, int offset, int whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    errno = ESPIPE;
    return -1;
}
int _read(int fd, char *data, int size)
{
    (void)fd;
    (void)data;
    (void)size;
    errno = ENOSYS;
    return -1;
}
int _write(int fd, char *data, int size)
{
    (void)fd;
    (void)data;
    (void)size;
    errno = ENOSYS;
    return -1;
}
int _fstat(int fd, struct stat *info)
{
    (void)fd;
    (void)info;
    errno = EBADF;
    return -1;
}
int _isatty(int fd)
{
    (void)fd;
    return 0;
}
