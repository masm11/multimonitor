/* Multi Monitor Plugin for Xfce
 *  Copyright (C) 2007 Yuuki Harano
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

#include "../config.h"
#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "types.h"
#include "battery.h"

#define NBATT 2

static int pow = -1;

static GList *list[NBATT] = { NULL, };

void battery_init(void)
{
    pow = open("/sys/class/power_supply", O_RDONLY);
}

void battery_read_data(gint type)
{
    if (pow < 0)
	return;
    
    int n = (type != TYPE_BATT_0);
    
    int cap = -1;
    char path[128];
    snprintf(path, sizeof path, "BAT%d/capacity", n);
    int fd = openat(pow, path, O_RDONLY);
    if (fd >= 0) {
	char buf[128];
	int s = read(fd, buf, sizeof buf);
	close(fd);
	if (s >= sizeof buf)
	    s = sizeof buf - 1;
	buf[s] = '\0';
	
	char *ep = NULL;
	cap = strtol(buf, &ep, 0);
	if (ep == NULL || *ep == '\0')
	    cap = -1;
    }
    
    gint *p = g_new0(gint, 1);
    *p = cap;
    list[n] = g_list_append(list[n], p);
}

void battery_draw_1(gint type, GdkPixmap *pix, GdkGC *bg, GdkGC *fg, GdkGC *err)
{
    int n = (type != TYPE_BATT_0);
    
    GList *lp = list[n];
    if (lp == NULL)
	return;
    
    gint x = 0;
    gint y = 0;
    gint w, h;
    gdk_pixmap_get_size(pix, &w, &h);
    
    gdk_draw_line(pix, bg,
	    x + w - 1,
	    y,
	    x + w - 1,
	    y + h - 1);
    
    int cap = *(gint *) lp->data;
    if (cap > 0) {
	gdk_draw_line(pix, fg,
		x + w - 2,
		y + h - h * cap / 100,
		x + w - 2,
		y + h - 1);
    }
}
