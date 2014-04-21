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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <cairo.h>
#include "types.h"
#include "list.h"
#include "draw.h"
#include "loadavg.h"

#define NAVG 3

static int dir = -1;
static GList *list[NAVG] = { NULL, };
static gdouble oldlevel[NAVG];
static gchar tooltip[NAVG][128];

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

void loadavg_draw_1(gint type, GdkPixbuf *pix, GdkColor *bg, GdkColor *fg, GdkColor *err)
{
    int n = (type - TYPE_LOADAVG_1);
    
    gdouble level = 1;
    
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    for (GList *lp = list[n]; lp != NULL; lp = g_list_next(lp)) {
	gdouble this_load = *(gdouble *) lp->data;
	gdouble this_level = ceil(this_load);
	if (this_level > level)
	    level = this_level;
    }
    
    if (oldlevel[n] != level) {
	oldlevel[n] = level;
	loadavg_draw_all(type, pix, bg, fg, err);
	return;
    }
    
    gdouble load = -1;
    GList *lp = list[n];
    if (lp != NULL)
	load = *(gdouble *) lp->data;
    
    if (load >= 0) {
	draw_line(pix, w - 1, 0, h - 1, bg);
	draw_line(pix, w - 1, h - h * load / level, h - 1, fg);
	
	for (gint i = 1; i < level; i++) {
	    draw_point(pix, w - 1, h - h * i / level, err);
	}
    } else {
	draw_line(pix, w - 1, 0, h - 1, err);
    }
}

void loadavg_draw_all(gint type, GdkPixbuf *pix, GdkColor *bg, GdkColor *fg, GdkColor *err)
{
    int n = (type - TYPE_LOADAVG_1);
    
    gdouble level = 1;
    
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    for (GList *lp = list[n]; lp != NULL; lp = g_list_next(lp)) {
	gdouble this_load = *(gdouble *) lp->data;
	gdouble this_level = ceil(this_load);
	if (this_level > level)
	    level = this_level;
    }
    
    gint x = w - 1;
    for (GList *lp = list[n]; lp != NULL && x >= 0; lp = g_list_next(lp), x--) {
	gdouble load = *(gdouble *) lp->data;
	
	if (load >= 0) {
	    draw_line(pix, x, 0, h - 1, bg);
	    draw_line(pix, x, h - h * load / level, h - 1, fg);
	    
	    for (gint i = 1; i < level; i++) {
		draw_point(pix, x, h - h * i / level, err);
	    }
	} else {
	    draw_line(pix, x, 0, h - 1, err);
	}
    }
    
    for ( ; x >= 0; x--)
	draw_line(pix, x, 0, h - 1, err);
}

void loadavg_discard_data(gint type, gint size)
{
    int n = (type - TYPE_LOADAVG_1);
    list[n] = list_truncate(list[n], size);
}

const gchar *loadavg_tooltip(gint type)
{
    gint n = type - TYPE_LOADAVG_1;
    gdouble load = -1;
    if (list[n] != NULL)
	load = *(gdouble *) list[n]->data;
    if (load < 0)
	return NULL;
    snprintf(tooltip[n], sizeof tooltip[n], "%.1f%%", load * 100);
    return tooltip[n];
}
