#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

gint sysfs_read_int(int dir, const char *pathfmt, ...)
{
    char path[128];
    
    va_list ap;
    va_start(ap, pathfmt);
    vsnprintf(path, sizeof path, pathfmt, ap);
    va_end(ap);
    
    int fd = openat(dir, path, O_RDONLY);
    
    if (fd < 0)
	return -1;
    
    char buf[128];
    int s = read(fd, buf, sizeof buf);
    close(fd);
    if (s >= sizeof buf)
	s = sizeof buf - 1;
    buf[s] = '\0';
    
    char *ep = NULL;
    gint val = strtol(buf, &ep, 0);
    if (!(ep != NULL && *ep != '\0'))
	val = -1;
    
    return val;
}
