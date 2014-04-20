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
#include <math.h>
#include "types.h"
#include "list.h"
#include "line.h"
#include "net.h"

#define NIF 6

struct log_t {
    gdouble logtx, logrx;
};

static int dir = -1;
static GList *list[NIF] = { NULL, };
static gint64 olddata[NIF][2] = { { 0, }, };	// [0]:rx, [1]:tx
static const char *ifnames[] = {
    "eth0",
    "eth1",
    "eth2",
    "wlan0",
    "ath0",
    "lo",
};

void net_init(void)
{
    dir = open("/proc/net", O_RDONLY);
}

void net_read_data(gint type)
{
    if (dir < 0)
	return;
    
    int n = type - TYPE_NET_ETH0;
    
    gint64 tx = -1, rx = -1;
    
    int fd = openat(dir, "dev", O_RDONLY);
    if (fd >= 0) {
	FILE *fp = fdopen(fd, "rt");
	
	char buf[1024];
	gint ver;
	
	fgets(buf, sizeof buf, fp);
	fgets(buf, sizeof buf, fp);
	if (strstr(buf, "compressed") != NULL)
	    ver = 3;
	else if (strstr(buf, "bytes") != NULL)
	    ver = 2;
	else
	    ver = 1;
	
	if (ver == 3) {
	    char ifcol[16];
	    snprintf(ifcol, sizeof ifcol, "%s:", ifnames[n]);
	    while (!feof(fp)) {
		char name[16];
		gint64 r, t;
		if (fscanf(fp, "%s %" G_GINT64_FORMAT " %*u %*u %*u %*u %*u %*u %*u %" G_GINT64_FORMAT " %*u %*u %*u %*u %*u %*u %*u",
				name, &r, &t) == 3) {
		    if (strcmp(name, ifcol) == 0) {
			rx = r - olddata[n][0];
			tx = t - olddata[n][1];
			olddata[n][0] = r;
			olddata[n][1] = t;
			break;
		    }
		}
	    }
	}
	
	fclose(fp);
    }
    
    struct log_t *p = g_new0(struct log_t, 1);
    if (rx < 0 || tx < 0) {
	p->logrx = -1;
	p->logtx = -1;
    } else {
	p->logrx = log(rx) / log(1024);
	p->logtx = log(tx) / log(1024);
	if (p->logrx < 0)
	    p->logrx = 0;
	if (p->logtx < 0)
	    p->logtx = 0;
    }
    list[n] = g_list_prepend(list[n], p);
}

void net_draw_1(gint type, GdkPixbuf *pix, GdkColor *bg, GdkColor *fg, GdkColor *err)
{
    int n = (type - TYPE_NET_ETH0);
    
    gdouble level = 1;
    
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    for (GList *lp = list[n]; lp != NULL; lp = g_list_next(lp)) {
	struct log_t *p = (struct log_t *) lp->data;
	gdouble lev_rx = ceil(p->logrx);
	gdouble lev_tx = ceil(p->logtx);
	if (lev_rx > level)
	    level = lev_rx;
	if (lev_tx > level)
	    level = lev_tx;
    }
    
    gint x = w - 1;
    for (GList *lp = list[n]; lp != NULL && x >= 0; lp = g_list_next(lp), x--) {
	struct log_t *p = (struct log_t *) lp->data;
	
	if (p->logrx >= 0 && p->logtx >= 0) {
	    draw_line(pix, x, 0, h - 1, bg);
	    draw_line(pix, x, h / 2 - h * p->logtx / level / 2, h / 2, fg);
	    draw_line(pix, x, h / 2, h / 2 + h * p->logrx / level / 2, fg);
	    
	    for (gint i = 0; i < level; i++)
		draw_line(pix, x, h / 2 - h * i / level / 2, h / 2 - h * i / level / 2, err);
	    for (gint i = 0; i < level; i++)
		draw_line(pix, x, h / 2 + h * i / level / 2, h / 2 + h * i / level / 2, err);
	} else {
	    draw_line(pix, x, 0, h - 1, err);
	}
    }
    
    for ( ; x >= 0; x--)
	draw_line(pix, x, 0, h - 1, err);
}

void net_discard_data(gint type, gint size)
{
    int n = (type - TYPE_NET_ETH0);
    list[n] = list_truncate(list[n], size);
}
