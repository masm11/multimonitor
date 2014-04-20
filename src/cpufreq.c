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
#include "cpufreq.h"

#define NCPU 4

static int dir = -1;
static int maxfreq[NCPU] = { -1, };
static GList *list[NCPU] = { NULL, };

void cpufreq_init(void)
{
    dir = open("/sys/devices/system/cpu", O_RDONLY);
    
    for (gint n = 0; n < NCPU; n++)
	maxfreq[n] = sysfs_read_int(dir, "cpu%d/cpufreq/cpuinfo_max_freq", n);
}

void cpufreq_read_data(gint type)
{
    if (dir < 0)
	return;
    
    int n = type - TYPE_CPUFREQ_0;
    
    int freq = sysfs_read_int(dir, "cpu%d/cpufreq/scaling_cur_freq", n);
    
    gint *p = g_new0(gint, 1);
    *p = freq;
    list[n] = g_list_prepend(list[n], p);
}

void cpufreq_draw_1(gint type, GdkPixbuf *pix, GdkColor *bg, GdkColor *fg, GdkColor *err)
{
    int n = (type - TYPE_CPUFREQ_0);
    
    int freq = -1;
    
    GList *lp = list[n];
    if (lp != NULL)
	freq = *(gint *) lp->data;
    
    gint w = gdk_pixbuf_get_width(pix);
    gint h = gdk_pixbuf_get_height(pix);
    
    if (freq >= 0) {
	draw_line(pix, w - 1, 0, h - 1, bg);
	draw_line(pix, w - 1, h - h * freq / maxfreq[n], h - 1, fg);
    } else {
	draw_line(pix, w - 1, 0, h - 1, err);
    }
}

void cpufreq_discard_data(gint type, gint size)
{
    int n = (type - TYPE_CPUFREQ_0);
    list[n] = list_truncate(list[n], size);
}
