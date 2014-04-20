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

#include <cairo.h>
#include <glib.h>
#include <gdk/gdk.h>
#include "line.h"

void draw_line(GdkPixbuf *pix,
	gint x, gint y1, gint y2,
	GdkColor *color)
{
    guint rowstride = gdk_pixbuf_get_rowstride(pix);
    guchar *p = gdk_pixbuf_get_pixels(pix) + rowstride * y1 + x * gdk_pixbuf_get_n_channels(pix);
    for (gint y = y1; y <= y2; y++) {
	p[0] = color->red >> 8;
	p[1] = color->green >> 8;
	p[2] = color->blue >> 8;
	p += rowstride;
    }
}

void draw_point(GdkPixbuf *pix,
	gint x, gint y,
	GdkColor *color)
{
    guint rowstride = gdk_pixbuf_get_rowstride(pix);
    guchar *p = gdk_pixbuf_get_pixels(pix) + rowstride * y + x * gdk_pixbuf_get_n_channels(pix);
    
    p[0] = color->red >> 8;
    p[1] = color->green >> 8;
    p[2] = color->blue >> 8;
}
