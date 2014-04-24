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
#include <math.h>
#include <cairo.h>
#include "types.h"
#include "list.h"
#include "draw.h"
#include "memory.h"

struct data_t {
    gint64 total;
    gint64 free;
    gint64 anon;
    gint64 buffers;
    gint64 cached;
    gint64 kernel;
};

static int dir = -1;
static GList *list = NULL;
static gchar tooltip[128];

void mem_init(void)
{
    dir = open("/proc", O_RDONLY);
}

void mem_read_data(gint type)
{
    if (dir < 0)
	return;
    
    struct data_t data;
    data.total = data.free = data.buffers = data.cached = data.kernel = 0;
    
    int fd = openat(dir, "meminfo", O_RDONLY);
    if (fd >= 0) {
	FILE *fp = fdopen(fd, "rt");
	
	while (!feof(fp)) {
	    char name[128];
	    gint64 value;
	    
	    if (fscanf(fp, "%s %"G_GINT64_FORMAT" kB", name, &value) == 2) {
		if (strcmp(name, "MemTotal:") == 0)
		    data.total = value;
		else if (strcmp(name, "MemFree:") == 0)
		    data.free = value;
		else if (strcmp(name, "Buffers:") == 0)
		    data.buffers = value;
		else if (strcmp(name, "Cached:") == 0)
		    data.cached = value;
		else if (strcmp(name, "AnonPages:") == 0)
		    data.anon = value;
	    }
	}
	
	fclose(fp);
	
	data.kernel = data.total - data.free - data.anon - data.buffers - data.cached;
    }
    
    struct data_t *p = g_new0(struct data_t, 1);
    *p = data;
    list = g_list_prepend(list, p);
}

static void draw_0(gint type, GdkPixbuf *pix, gint w, gint h, struct data_t *p, gint x)
{
    if (p != NULL && p->total >= 0) {
	draw_line(pix, x, 0, h - 1, color_bg_normal);
	draw_line(pix, x, h - h * (p->anon + p->buffers + p->cached + p->kernel) / p->total, h - 1, color_fg_anon);
	draw_line(pix, x, h - h * (p->buffers + p->cached + p->kernel) / p->total, h - 1, color_fg_buffers);
	draw_line(pix, x, h - h * (p->cached + p->kernel) / p->total, h - 1, color_fg_cached);
	draw_line(pix, x, h - h * (p->kernel) / p->total, h - 1, color_fg_kernel);
    } else {
	draw_line(pix, x, 0, h - 1, color_err);
    }
}

void mem_draw_1(gint type, GdkPixbuf *pix)
{
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    struct data_t *p = NULL;
    GList *lp = list;
    if (lp != NULL)
	p = (struct data_t *) lp->data;
    
    draw_0(type, pix, w, h, p, w - 1);
}

void mem_draw_all(gint type, GdkPixbuf *pix)
{
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    gint x = w - 1;
    for (GList *lp = list; lp != NULL && x >= 0; lp = g_list_next(lp), x--) {
	struct data_t *p = (struct data_t *) lp->data;
	
	draw_0(type, pix, w, h, p, x);
    }
    
    for ( ; x >= 0; x--)
	draw_0(type, pix, w, h, NULL, x);
}

void mem_discard_data(gint type, gint size)
{
    list = list_truncate(list, size);
}

const gchar *mem_tooltip(gint type)
{
    struct data_t *p = NULL;
    if (list != NULL)
	p = list->data;
    if (p == NULL)
	return NULL;
    snprintf(tooltip, sizeof tooltip,
	    "Anon:%.1fMB\nBuffers:%.1fMB\nCached:%.1fMB\nKernel:%.1fMB",
	    (gdouble) p->anon / 1024,
	    (gdouble) p->buffers / 1024,
	    (gdouble) p->cached / 1024,
	    (gdouble) p->kernel / 1024);
    return tooltip;
}
