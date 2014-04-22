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
#include "list.h"
#include "draw.h"
#include "cpuload.h"

#define NCPU 4
#define NDATA 10

static int dir = -1;
static GList *list[NCPU] = { NULL, };
static gboolean read_flag[NCPU] = { FALSE, };
static guint64 olddata[NCPU][NDATA];
static gchar tooltip[NCPU][128];

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
	    guint64 data[NDATA];
	    if (fscanf(fp, "%s %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT"",
			    name,
			    &data[0], &data[1], &data[2], &data[3], &data[4], &data[5], &data[6], &data[7], &data[8], &data[9]) == 11) {
		if (strcmp(name, wanted) == 0) {
		    guint64 busy = 0;
		    guint64 idle = 0;
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
    
    // 最初のデータはゴミなので捨てる。
    if (!read_flag[n]) {
	read_flag[n] = TRUE;
	load = -1;
    }
    
    gdouble *p = g_new0(gdouble, 1);
    *p = load;
    list[n] = g_list_prepend(list[n], p);
}

void cpuload_draw_1(gint type, GdkPixbuf *pix)
{
    int n = (type - TYPE_CPULOAD_0);
    
    double load = -1;
    
    GList *lp = list[n];
    if (lp != NULL)
	load = *(gdouble *) lp->data;
    
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    if (load >= 0) {
	draw_line(pix, w - 1, 0, h - 1, color_bg_normal);
	draw_line(pix, w - 1, h - h * load, h - 1, color_fg_normal);
    } else {
	draw_line(pix, w - 1, 0, h - 1, color_err);
    }
}

void cpuload_draw_all(gint type, GdkPixbuf *pix)
{
    int n = (type - TYPE_CPULOAD_0);
    
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    gint x = w - 1;
    for (GList *lp = list[n]; lp != NULL && x >= 0; lp = g_list_next(lp), x--) {
	gdouble load = *(gdouble *) lp->data;
	
	if (load >= 0) {
	    draw_line(pix, x, 0, h - 1, color_bg_normal);
	    draw_line(pix, x, h - h * load, h - 1, color_fg_normal);
	} else {
	    draw_line(pix, x, 0, h - 1, color_err);
	}
    }
    
    for ( ; x >= 0; x--)
	draw_line(pix, x, 0, h - 1, color_err);
}

void cpuload_discard_data(gint type, gint size)
{
    int n = (type - TYPE_CPULOAD_0);
    list[n] = list_truncate(list[n], size);
}

const gchar *cpuload_tooltip(gint type)
{
    gint n = type - TYPE_CPULOAD_0;
    gdouble load = -1;
    if (list[n] != NULL)
	load = *(gdouble *) list[n]->data;
    if (load < 0)
	return NULL;
    snprintf(tooltip[n], sizeof tooltip[n], "%.1f%%", load * 100);
    return tooltip[n];
}
