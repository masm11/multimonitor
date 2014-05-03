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

#include <string.h>
#include <cairo.h>
#include <glib.h>
#include <gdk/gdk.h>
#include "draw.h"

#define DEFINE_COLOR(name, r, g, b) \
  static GdkColor name = { .red = r, .green = g, .blue = b }; \
  GdkColor *color_##name = &name;

DEFINE_COLOR(err,       0x8080, 0x8080, 0x8080)
DEFINE_COLOR(bg_normal, 0x0000, 0x0000, 0x0000)
DEFINE_COLOR(fg_normal, 0xffff, 0x0000, 0x0000)
DEFINE_COLOR(bg_charge, 0x0000, 0x4040, 0x0000)
DEFINE_COLOR(fg_charge, 0xffff, 0x4040, 0x0000)
DEFINE_COLOR(fg_tx,     0xffff, 0x0000, 0x0000)
DEFINE_COLOR(fg_rx,     0x0000, 0xffff, 0x0000)
DEFINE_COLOR(text,      0xffff, 0xffff, 0xffff)
DEFINE_COLOR(fg_anon,   0xffff, 0x0000, 0x0000)
DEFINE_COLOR(fg_kernel, 0xffff, 0x8080, 0x8080)
DEFINE_COLOR(fg_cached, 0x8080, 0x0000, 0x0000)
DEFINE_COLOR(fg_buffers,0x8080, 0x4040, 0x0000)

static inline gint clip(gint a, gint max)
{
    if (a < 0)
	a = 0;
    if (a > max)
	a = max;
    return a;
}

void draw_line(GdkPixbuf *pix,
	gint x, gint y1, gint y2,
	GdkColor *color)
{
    x = clip(x, gdk_pixbuf_get_width(pix) - 1);
    y1 = clip(y1, gdk_pixbuf_get_height(pix) - 1);
    y2 = clip(y2, gdk_pixbuf_get_height(pix) - 1);
    
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
    x = clip(x, gdk_pixbuf_get_width(pix) - 1);
    y = clip(y, gdk_pixbuf_get_height(pix) - 1);
    
    guint rowstride = gdk_pixbuf_get_rowstride(pix);
    guchar *p = gdk_pixbuf_get_pixels(pix) + rowstride * y + x * gdk_pixbuf_get_n_channels(pix);
    
    p[0] = color->red >> 8;
    p[1] = color->green >> 8;
    p[2] = color->blue >> 8;
}

void draw_shift(GdkPixbuf *pix)
{
    guchar *pixels = gdk_pixbuf_get_pixels(pix);
    gint rowstride = gdk_pixbuf_get_rowstride(pix);
    gint nch = gdk_pixbuf_get_n_channels(pix);
    guchar *dst = pixels;
    guchar *src = dst + nch;
    guint len = nch * (gdk_pixbuf_get_width(pix) - 1);
    for (gint y = 0; y < gdk_pixbuf_get_height(pix); y++) {
	memmove(dst, src, len);
	src += rowstride;
	dst += rowstride;
    }
}
