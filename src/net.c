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
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "types.h"
#include "list.h"
#include "draw.h"
#include "sysfs.h"
#include "net.h"

#define NIF 6

struct log_t {
    gdouble logtx, logrx;
};

static int dir = -1;
static int procdir = -1;
static GList *list[NIF] = { NULL, };
static gint64 olddata[NIF][2] = { { 0, }, };	// [0]:rx, [1]:tx
static gint64 lastdata[NIF][2] = { { 0, }, };	// [0]:rx, [1]:tx
static gdouble oldlevel[NIF] = { 0, };
static gboolean read_flag[NIF] = { FALSE, };
static gchar tooltip[NIF][128];
static const char *ifnames[] = {
    "eth0",
    "eth1",
    "eth2",
    "wlan0",
    "ath0",
    "lo",
};

static gint get_addr4(const gchar *ifname, gchar *buf, gint bufsiz);
static gint get_addr6(const gchar *ifname, gchar *buf, gint bufsiz);
static gint append_bps(const gchar *label, gint64 bps, gchar *buf, gint bufsiz);

void net_init(void)
{
    dir = open("/sys/class/net", O_RDONLY);
    procdir = open("/proc/net", O_RDONLY);
}

void net_read_data(gint type)
{
    if (dir < 0)
	return;
    
    int n = type - TYPE_NET_ETH0;
    
    gint64 r = sysfs_read_64(dir, "%s/statistics/rx_bytes", ifnames[n]);
    gint64 t = sysfs_read_64(dir, "%s/statistics/tx_bytes", ifnames[n]);
    
    gint64 rx = -1;
    gint64 tx = -1;
    if (r >= 0 && t >= 0) {
	rx = r - olddata[n][0];
	tx = t - olddata[n][1];
    }
    olddata[n][0] = r;
    olddata[n][1] = t;
    
    if (!read_flag[n]) {
	read_flag[n] = TRUE;
	rx = tx = -1;
    }
    
    lastdata[n][0] = rx;
    lastdata[n][1] = tx;
    
    struct log_t *p = g_new0(struct log_t, 1);
    if (rx < 0 || tx < 0) {
	p->logrx = -1;
	p->logtx = -1;
    } else {
	p->logrx = log(rx * 8) / log(1024);
	p->logtx = log(tx * 8) / log(1024);
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
    
    GdkColor fg2 = {
	.red = 0,
	.green = 65535,
	.blue = 0,
    };
    
    gdouble level = 1;
    
    for (GList *lp = list[n]; lp != NULL; lp = g_list_next(lp)) {
	struct log_t *p = (struct log_t *) lp->data;
	gdouble lev_rx = ceil(p->logrx);
	gdouble lev_tx = ceil(p->logtx);
	if (lev_rx > level)
	    level = lev_rx;
	if (lev_tx > level)
	    level = lev_tx;
    }
    
    if (oldlevel[n] != level) {
	oldlevel[n] = level;
	net_draw_all(type, pix, bg, fg, err);
	return;
    }
    
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    struct log_t *p = NULL;
    GList *lp = list[n];
    if (lp != NULL)
	p = lp->data;
    
    if (p != NULL && p->logrx >= 0 && p->logtx >= 0) {
	draw_line(pix, w - 1, 0, h - 1, bg);
	draw_line(pix, w - 1, h / 2 - h * p->logtx / level / 2, h / 2, fg);
	draw_line(pix, w - 1, h / 2, h / 2 + h * p->logrx / level / 2, &fg2);
	
	for (gint i = 0; i < level; i++)
	    draw_point(pix, w - 1, h / 2 - h * i / level / 2, err);
	for (gint i = 0; i < level; i++)
	    draw_point(pix, w - 1, h / 2 + h * i / level / 2, err);
    } else {
	draw_line(pix, w - 1, 0, h - 1, err);
    }
}

void net_draw_all(gint type, GdkPixbuf *pix, GdkColor *bg, GdkColor *fg, GdkColor *err)
{
    int n = (type - TYPE_NET_ETH0);
    
    GdkColor fg2 = {
	.red = 0,
	.green = 65535,
	.blue = 0,
    };
    
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
	    draw_line(pix, x, h / 2, h / 2 + h * p->logrx / level / 2, &fg2);
	    
	    for (gint i = 0; i < level; i++)
		draw_point(pix, x, h / 2 - h * i / level / 2, err);
	    for (gint i = 0; i < level; i++)
		draw_point(pix, x, h / 2 + h * i / level / 2, err);
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

const gchar *net_tooltip(gint type)
{
    gint n = type - TYPE_NET_ETH0;
    
    if (lastdata[n][0] < 0 || lastdata[n][1] < 0)
	return NULL;
    
    gchar *buf = tooltip[n];
    gint bufsiz = sizeof tooltip[n];
    
    gint siz;
    
    siz = get_addr4(ifnames[n], buf, bufsiz);
    buf += siz;
    bufsiz -= siz;
    
    siz = get_addr6(ifnames[n], buf, bufsiz);
    buf += siz;
    bufsiz -= siz;
    
    siz = append_bps("Tx", lastdata[n][1] * 8, buf, bufsiz);
    buf += siz;
    bufsiz -= siz;
    
    siz = append_bps("Rx", lastdata[n][0] * 8, buf, bufsiz);
    buf += siz;
    bufsiz -= siz;
    
    // 最後の \n を削除。
    char *p = tooltip[n];
    p += strlen(p);
    if (*--p == '\n')
	*p = '\0';
    
    return tooltip[n];
}

static gint get_addr4(const gchar *ifname, gchar *buf, gint bufsiz)
{
    struct ifreq ifr;
    int s;
    
    s = socket(PF_INET, SOCK_DGRAM, 0);
    if (s == -1)
	return 0;
    
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;
    
    if (ioctl(s, SIOCGIFADDR, &ifr) == -1) {
	close(s);
	return 0;
    }
    
    struct in_addr addr = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr;
    
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;
    
    if (ioctl(s, SIOCGIFNETMASK, &ifr) == -1) {
	close(s);
	return 0;
    }
    
    const guchar *m = (const guchar *) &((struct sockaddr_in *) &ifr.ifr_netmask)->sin_addr;
    gint prefixlen = 0;
    for (gint i = 0; i < 4; i++) {
	for (gint j = 0; j < 8; j++) {
	    if (m[i] & (0x80 >> j))
		prefixlen++;
	}
    }
    
    close(s);
    
    return snprintf(buf, bufsiz, "%d.%d.%d.%d/%d\n",
	    ((const guchar *) &addr)[0],
	    ((const guchar *) &addr)[1],
	    ((const guchar *) &addr)[2],
	    ((const guchar *) &addr)[3],
	    prefixlen);
}

static gint get_addr6(const gchar *ifname, gchar *buf, gint bufsiz)
{
    gint orig_bufsiz = bufsiz;
    
    if (procdir < 0)
	return 0;
    
    int fd = openat(procdir, "if_inet6", O_RDONLY);
    if (fd < 0)
	return 0;
    
    FILE *fp = fdopen(fd, "rt");
    
    while (!feof(fp)) {
	gchar addrstr[40];
	gint idx;
	gint prefixlen;
	gint scope;
	gint dad_status;
	gchar devname[16];
	
	if (fscanf(fp, "%40s %x %x %x %x %s",
			addrstr, &idx, &prefixlen, &scope, &dad_status, devname) != 6)
	    continue;
	if (strcmp(devname, ifname) != 0)
	    continue;
	if (strlen(addrstr) != 32)
	    continue;
	
	gchar tmpaddr[128];
	gchar *s = addrstr;
	gchar *d = tmpaddr;
	for (gint i = 0; i < 8; i++) {
	    *d++ = *s++;
	    *d++ = *s++;
	    *d++ = *s++;
	    *d++ = *s++;
	    if (i != 7)
		*d++ = ':';
	    else
		*d++ = '\0';
	}
	
	// 先頭の 0 を取り除き、:0:0:… を :: に圧縮するため、
	// inet_pton と inet_ntop で変換する。
	struct in6_addr addr;
	if (inet_pton(AF_INET6, tmpaddr, &addr) <= 0)
	    continue;
	if (inet_ntop(AF_INET6, &addr, tmpaddr, sizeof tmpaddr) == NULL)
	    continue;
	
	gint size = snprintf(buf, bufsiz, "%s/%d\n", tmpaddr, prefixlen);
	buf += size;
	bufsiz -= size;
    }
    
    fclose(fp);
    
    return orig_bufsiz - bufsiz;
}

static gint append_bps(const gchar *label, gint64 bps, gchar *buf, gint bufsiz)
{
    if (bps < 1000)
	return snprintf(buf, bufsiz, "%s:%dbps\n", label, (gint) bps);
    else if (bps < 1000000)
	return snprintf(buf, bufsiz, "%s:%.1fKbps\n", label, (gdouble) bps / 1000);
    else if (bps < 1000000000)
	return snprintf(buf, bufsiz, "%s:%.1fMbps\n", label, (gdouble) bps / 1000 / 1000);
    else
	return snprintf(buf, bufsiz, "%s:%.1fGbps\n", label, (gdouble) bps / 1000 / 1000 / 1000);
}
