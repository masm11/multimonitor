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
#include <math.h>
#include "types.h"
#include "list.h"
#include "loadavg.h"

#define NAVG 3

static int dir = -1;
static GList *list[NAVG] = { NULL, };

void loadavg_init(void)
{
    dir = open("/proc", O_RDONLY);
}

void loadavg_read_data(gint type)
{
    if (dir < 0)
	return;
    
    int n = type - TYPE_LOADAVG_1;
    
    double load = -1;
    
    int fd = openat(dir, "loadavg", O_RDONLY);
    if (fd >= 0) {
	FILE *fp = fdopen(fd, "rt");
	
	switch (type) {
	case TYPE_LOADAVG_1:
	    fscanf(fp, "%lf", &load);
	    break;
	case TYPE_LOADAVG_5:
	    fscanf(fp, "%*f %lf", &load);
	    break;
	case TYPE_LOADAVG_15:
	    fscanf(fp, "%*f %*f %lf", &load);
	    break;
	default:
	    load = -1;
	}
	
	fclose(fp);
    }
    
    gdouble *p = g_new0(gdouble, 1);
    *p = load;
    list[n] = g_list_prepend(list[n], p);
}

void loadavg_draw_1(gint type, GdkPixmap *pix, GdkGC *bg, GdkGC *fg, GdkGC *err)
{
    int n = (type - TYPE_LOADAVG_1);
    
    double load = -1;
    
    GList *lp = list[n];
    if (lp != NULL)
	load = *(gdouble *) lp->data;
    
    gint w, h;
    gdk_pixmap_get_size(pix, &w, &h);
    
    if (load >= 0) {
	gdouble level = ceil(load);
	
	gdk_draw_line(pix, bg,
		w - 1, 0,
		w - 1, h - 1);
	
	gdk_draw_line(pix, fg,
		w - 1, h - h * load / level,
		w - 1, h - 1);
	
	for (gint i = 1; i < level; i++) {
	    gdk_draw_line(pix, err,
		    w - 1, h - h * i / level,
		    w - 1, h - h * i / level);
	}
	
    } else {
	gdk_draw_line(pix, err,
		w - 1, 0,
		w - 1, h - 1);
    }
}

void loadavg_discard_data(gint type, gint size)
{
    int n = (type - TYPE_LOADAVG_1);
    list[n] = list_truncate(list[n], size);
}
