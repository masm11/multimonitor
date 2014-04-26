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
#include "types.h"
#include "list.h"
#include "draw.h"
#include "sysfs.h"
#include "disk.h"

#define NDISK 6

struct data_t {
    gdouble logr, logw;
};

static int dir = -1;
static GList *list[NDISK] = { NULL, };
static gint64 olddata[NDISK][2] = { { 0, }, };	// [0]:r, [1]:w
static gint64 lastdata[NDISK][2] = { { 0, }, };	// [0]:r, [1]:w
static gdouble oldlevel[NDISK] = { 0, };
static gboolean read_flag[NDISK] = { FALSE, };
static gchar tooltip[NDISK][128];
static const char *devnames[] = {
    "sda",
    "sdb",
    "sdc",
    "hda",
    "hdb",
    "hdc",
};

void disk_init(void)
{
    dir = open("/proc", O_RDONLY);
}

void disk_read_data(gint type)
{
    if (dir < 0)
	return;
    
    int n = type - TYPE_DISK_SDA;
    
    gint64 rsec = -1, wsec = -1;
    gint64 r = -1, w = -1;
    
    int fd = openat(dir, "diskstats", O_RDONLY);
    if (fd >= 0) {
	FILE *fp = fdopen(fd, "rt");
	
	while (!feof(fp)) {
	    char dev[16];
	    gint64 rr, ww;
	    if (fscanf(fp, "%*d %*d %s %*d %*d %"G_GINT64_FORMAT" %*d %*d %*d %"G_GINT64_FORMAT" %*d %*d %*d %*d",
			    dev, &rr, &ww) == 3) {
		if (strcmp(dev, devnames[n]) == 0) {
		    r = rr;
		    w = ww;
		    break;
		}
	    }
	}
	
	fclose(fp);
    }
    
    if (r >= 0 && w >= 0) {
	rsec = r - olddata[n][0];
	wsec = w - olddata[n][1];
    }
    olddata[n][0] = r;
    olddata[n][1] = w;
    
    // 最初のデータはゴミなので捨てる。
    if (!read_flag[n]) {
	read_flag[n] = TRUE;
	rsec = wsec = -1;
    }
    
    if (rsec >= 0 && wsec >= 0) {
	rsec *= 512;
	wsec *= 512;
    }
    
    lastdata[n][0] = rsec;
    lastdata[n][1] = wsec;
    
    struct data_t *p = g_new0(struct data_t, 1);
    if (rsec < 0 || wsec < 0) {
	p->logr = -1;
	p->logw = -1;
    } else {
	p->logr = log(rsec) / log(1024);
	p->logw = log(wsec) / log(1024);
	if (p->logr < 0)
	    p->logr = 0;
	if (p->logw < 0)
	    p->logw = 0;
    }
    list[n] = g_list_prepend(list[n], p);
}

static void draw_0(gint type, GdkPixbuf *pix, gint w, gint h, struct data_t *p, gdouble level, gint x)
{
    if (p != NULL && p->logr >= 0 && p->logw >= 0) {
	draw_line(pix, x, 0, h - 1, color_bg_normal);
	draw_line(pix, x, h / 2 - h * p->logw / level / 2, h / 2, color_fg_tx);
	draw_line(pix, x, h / 2, h / 2 + h * p->logr / level / 2, color_fg_rx);
	
	for (gint i = 0; i < level; i++)
	    draw_point(pix, x, h / 2 - h * i / level / 2, color_err);
	for (gint i = 0; i < level; i++)
	    draw_point(pix, x, h / 2 + h * i / level / 2, color_err);
    } else {
	draw_line(pix, x, 0, h - 1, color_err);
    }
}

void disk_draw_1(gint type, GdkPixbuf *pix)
{
    int n = (type - TYPE_DISK_SDA);
    
    gdouble level = 1;
    
    for (GList *lp = list[n]; lp != NULL; lp = g_list_next(lp)) {
	struct data_t *p = (struct data_t *) lp->data;
	gdouble lev_r = ceil(p->logr);
	gdouble lev_w = ceil(p->logw);
	if (lev_r > level)
	    level = lev_r;
	if (lev_w > level)
	    level = lev_w;
    }
    
    if (oldlevel[n] != level) {
	oldlevel[n] = level;
	disk_draw_all(type, pix);
	return;
    }
    
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    struct data_t *p = NULL;
    GList *lp = list[n];
    if (lp != NULL)
	p = lp->data;
    
    draw_0(type, pix, w, h, p, level, w - 1);
}

void disk_draw_all(gint type, GdkPixbuf *pix)
{
    int n = (type - TYPE_DISK_SDA);
    
    gdouble level = 1;
    
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    for (GList *lp = list[n]; lp != NULL; lp = g_list_next(lp)) {
	struct data_t *p = (struct data_t *) lp->data;
	gdouble lev_r = ceil(p->logr);
	gdouble lev_w = ceil(p->logw);
	if (lev_r > level)
	    level = lev_r;
	if (lev_w > level)
	    level = lev_w;
    }
    
    gint x = w - 1;
    for (GList *lp = list[n]; lp != NULL && x >= 0; lp = g_list_next(lp), x--) {
	struct data_t *p = (struct data_t *) lp->data;
	
	draw_0(type, pix, w, h, p, level, x);
    }
    
    for ( ; x >= 0; x--)
	draw_0(type, pix, w, h, NULL, level, x);
}

void disk_discard_data(gint type, gint size)
{
    int n = (type - TYPE_DISK_SDA);
    list[n] = list_truncate(list[n], size);
}

static gint append_Bps(const gchar *label, gdouble Bps, gchar *buf, gint bufsiz)
{
    if (Bps < 1000)
	return snprintf(buf, bufsiz, "%s:%dB/s\n", label, (gint) Bps);
    else if (Bps < 1000000)
	return snprintf(buf, bufsiz, "%s:%.1fKB/s\n", label, (gdouble) Bps / 1000);
    else if (Bps < 1000000000)
	return snprintf(buf, bufsiz, "%s:%.1fMB/s\n", label, (gdouble) Bps / 1000 / 1000);
    else
	return snprintf(buf, bufsiz, "%s:%.1fGB/s\n", label, (gdouble) Bps / 1000 / 1000 / 1000);
}

const gchar *disk_tooltip(gint type)
{
    gint n = type - TYPE_DISK_SDA;
    
    if (lastdata[n][0] < 0 || lastdata[n][1] < 0)
	return NULL;
    
    gchar *buf = tooltip[n];
    gint bufsiz = sizeof tooltip[n];
    gint s;
    
    s = append_Bps("Write", lastdata[n][1], buf, bufsiz);
    buf += s;
    bufsiz -= s;
    
    s = append_Bps("Read", lastdata[n][0], buf, bufsiz);
    buf += s;
    bufsiz -= s;
    
    gint len = strlen(tooltip[n]);
    if (len >= 1 && tooltip[n][len - 1] == '\n')
	tooltip[n][len - 1] = '\0';
    
    return tooltip[n];
}
