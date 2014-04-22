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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "types.h"
#include "sysfs.h"
#include "list.h"
#include "draw.h"
#include "battery.h"

#define NBATT 2

static int pow = -1;

struct data_t {
    gboolean charging;
    gint capacity;
};

static GList *list[NBATT] = { NULL, };
static gchar tooltip[NBATT][128];

void battery_init(void)
{
    pow = open("/sys/class/power_supply", O_RDONLY);
}

void battery_read_data(gint type)
{
    if (pow < 0)
	return;
    
    int n = type - TYPE_BATT_0;
    
    int cap = sysfs_read_int(pow, "BAT%d/capacity", n);
    char status[16];
    sysfs_read_str(pow, status, sizeof status, "BAT%d/status", n);
    
    struct data_t *p = g_new0(struct data_t, 1);
    p->charging = (strcasecmp(status, "charging") == 0);
    p->capacity = cap;
    list[n] = g_list_prepend(list[n], p);
}

static void draw_0(gint type, GdkPixbuf *pix, gint w, gint h, struct data_t *p, gint x)
{
    if (p != NULL && p->capacity >= 0) {
	GdkColor *bg = color_bg_normal, *fg = color_fg_normal;
	if (p->charging) {
	    bg = color_bg_charge;
	    fg = color_fg_charge;
	}
	draw_line(pix, x, 0, h - 1, bg);
	draw_line(pix, x, h - h * p->capacity / 100, h - 1, fg);
    } else {
	draw_line(pix, x, 0, h - 1, color_err);
    }
}

void battery_draw_1(gint type, GdkPixbuf *pix)
{
    int n = type - TYPE_BATT_0;
    
    struct data_t *p = NULL;
    
    GList *lp = list[n];
    if (lp != NULL)
	p = (struct data_t *) lp->data;
    
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    draw_0(type, pix, w, h, p, w - 1);
}

void battery_draw_all(gint type, GdkPixbuf *pix)
{
    int n = type - TYPE_BATT_0;
    
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    gint x = w - 1;
    for (GList *lp = list[n]; lp != NULL && x >= 0; lp = g_list_next(lp), x--) {
	struct data_t *p = (struct data_t *) lp->data;
	
	draw_0(type, pix, w, h, p, x);
    }
    
    for ( ; x >= 0; x--)
	draw_0(type, pix, w, h, NULL, x);
}

void battery_discard_data(gint type, gint size)
{
    int n = type - TYPE_BATT_0;
    list[n] = list_truncate(list[n], size);
}

const gchar *battery_tooltip(gint type)
{
    gint n = type - TYPE_BATT_0;
    struct data_t *p = NULL;
    if (list[n] != NULL)
	p = (struct data_t *) list[n]->data;
    if (p == NULL || p->capacity < 0)
	return NULL;
    snprintf(tooltip[n], sizeof tooltip[n], "%d%%\n%scharging",
	    p->capacity, p->charging ? "" : "dis");
    return tooltip[n];
}
