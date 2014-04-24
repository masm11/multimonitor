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

#ifndef DRAW_H__INCLUDED
#define DRAW_H__INCLUDED

extern GdkColor *color_err;
extern GdkColor *color_bg_normal;
extern GdkColor *color_fg_normal;
extern GdkColor *color_bg_charge;
extern GdkColor *color_fg_charge;
extern GdkColor *color_fg_tx;
extern GdkColor *color_fg_rx;
extern GdkColor *color_text;
extern GdkColor *color_fg_kernel;
extern GdkColor *color_fg_cached;
extern GdkColor *color_fg_buffers;

void draw_line(GdkPixbuf *pix,
	gint x, gint y1, gint y2,
	GdkColor *color);

void draw_point(GdkPixbuf *pix,
	gint x, gint y,
	GdkColor *color);

void draw_shift(GdkPixbuf *pix);

#endif	/* ifndef DRAW_H__INCLUDED */
