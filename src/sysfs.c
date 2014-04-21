/* Multi Monitor Plugin for Xfce
 *  Copyright (C) 2007,2014 Yuuki Harano
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

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

gint64 sysfs_read_64(int dir, const char *pathfmt, ...)
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
    gint64 val = strtoll(buf, &ep, 0);
    if (!(ep != NULL && *ep != '\0'))
	val = -1;
    
    return val;
}
