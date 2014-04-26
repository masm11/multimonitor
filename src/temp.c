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
#include "types.h"
#include "sysfs.h"
#include "list.h"
#include "draw.h"
#include "temp.h"

static int dir = -1;
static gdouble max = 0;
static GList *list = { NULL, };
static gchar tooltip[128];

void temp_init(void)
{
    dir = open("/sys/devices/platform/coretemp.0", O_RDONLY);
    
    max = (gdouble) sysfs_read_int(dir, "temp1_max") / 1000;
}

void temp_read_data(gint type)
{
    if (dir < 0)
	return;
    
    gdouble temp = (gdouble) sysfs_read_int(dir, "temp1_input") / 1000;
    
    gdouble *p = g_new0(gdouble, 1);
    *p = temp;
    list = g_list_prepend(list, p);
}

static void draw_0(gint type, GdkPixbuf *pix, gint w, gint h, gdouble temp, gdouble max, gint x)
{
    if (temp >= 0) {
	draw_line(pix, x, 0, h - 1, color_bg_normal);
	draw_line(pix, x, h - h * temp / max, h - 1, color_fg_normal);
    } else {
	draw_line(pix, x, 0, h - 1, color_err);
    }
}

void temp_draw_1(gint type, GdkPixbuf *pix)
{
    gdouble temp = -1;
    
    GList *lp = list;
    if (lp != NULL)
	temp = *(gdouble *) lp->data;
    
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    draw_0(type, pix, w, h, temp, max, w - 1);
}

void temp_draw_all(gint type, GdkPixbuf *pix)
{
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    gint x = w - 1;
    for (GList *lp = list; lp != NULL && x >= 0; lp = g_list_next(lp), x--) {
	gdouble temp = *(gdouble *) lp->data;
	
	draw_0(type, pix, w, h, temp, max, x);
    }
    
    for ( ; x >= 0; x--)
	draw_0(type, pix, w, h, -1, max, x);
}

void temp_discard_data(gint type, gint size)
{
    list = list_truncate(list, size);
}

const gchar *temp_tooltip(gint type)
{
    gdouble temp = -1;
    if (list != NULL)
	temp = *(gdouble *) list->data;
    if (temp < 0)
	return NULL;
    
    snprintf(tooltip, sizeof tooltip, "%.1fC", temp);
    
    return tooltip;
}
