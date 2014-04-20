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
