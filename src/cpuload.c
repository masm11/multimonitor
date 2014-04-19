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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "types.h"
#include "battery.h"

#define NCPU 4
#define NDATA 8

static int dir = -1;
static GList *list[NCPU] = { NULL, };
static gint olddata[NCPU][NDATA];

void cpuload_init(void)
{
    dir = open("/proc", O_RDONLY);
}

void cpuload_read_data(gint type)
{
    if (dir < 0)
	return;
    
    gint n = type - TYPE_CPULOAD_0;
    
    char wanted[32];
    snprintf(wanted, sizeof wanted, "cpu%d", n);
    
    gdouble load = -1;
    int fd = openat(dir, "stat", O_RDONLY);
    if (fd >= 0) {
	FILE *fp = fdopen(fd, "rt");
	char name[32];
	
	while (!feof(fp)) {
	    gint data[NDATA];
	    if (fscanf(fp, "%s %d %d %d %d %d %d %d %d",
			    name,
			    &data[0], &data[1], &data[2], &data[3], &data[4], &data[5], &data[6], &data[7]) == 9) {
		if (strcmp(name, wanted) == 0) {
		    gint busy = 0;
		    gint idle = 0;
		    for (gint i = 0; i < NDATA; i++) {
			if (i != 3)
			    busy += data[i] - olddata[n][i];
			else
			    idle += data[i] - olddata[n][i];
			olddata[n][i] = data[i];
		    }
		    load = (gdouble) busy / (busy + idle);
		    break;
		}
	    }
	}
	
	fclose(fp);
    }
    
    gdouble *p = g_new0(gdouble, 1);
    *p = load;
    list[n] = g_list_prepend(list[n], p);
}

void cpuload_draw_1(gint type, GdkPixmap *pix, GdkGC *bg, GdkGC *fg, GdkGC *err)
{
    int n = (type - TYPE_CPULOAD_0);
    
    double load = -1;
    
    GList *lp = list[n];
    if (lp != NULL)
	load = *(gdouble *) lp->data;
    
    gint w, h;
    gdk_pixmap_get_size(pix, &w, &h);
    
    if (load >= 0) {
	gdk_draw_line(pix, bg,
		w - 1, 0,
		w - 1, h - 1);
	
	gdk_draw_line(pix, fg,
		w - 2, h - h * load,
		w - 2, h - 1);
    } else {
	gdk_draw_line(pix, err,
		w - 1, 0,
		w - 1, h - 1);
    }
}
