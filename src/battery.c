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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "types.h"
#include "sysfs.h"
#include "battery.h"

#define NBATT 2

static int pow = -1;

static GList *list[NBATT] = { NULL, };

void battery_init(void)
{
    pow = open("/sys/class/power_supply", O_RDONLY);
}

void battery_read_data(gint type)
{
    if (pow < 0)
	return;
    
    int n = (type != TYPE_BATT_0);
    
    int cap = sysfs_read_int(pow, "BAT%d/capacity", n);
    
    gint *p = g_new0(gint, 1);
    *p = cap;
    list[n] = g_list_prepend(list[n], p);
}

void battery_draw_1(gint type, GdkPixmap *pix, GdkGC *bg, GdkGC *fg, GdkGC *err)
{
    int n = (type != TYPE_BATT_0);
    
    int cap = -1;
    
    GList *lp = list[n];
    if (lp != NULL)
	cap = *(gint *) lp->data;
    
    gint w, h;
    gdk_pixmap_get_size(pix, &w, &h);
    
    if (cap >= 0) {
	gdk_draw_line(pix, bg,
		w - 1, 0,
		w - 1, h - 1);
	
	gdk_draw_line(pix, fg,
		w - 2, h - h * cap / 100,
		w - 2, h - 1);
    } else {
	gdk_draw_line(pix, err,
		w - 1, 0,
		w - 1, h - 1);
    }
}
