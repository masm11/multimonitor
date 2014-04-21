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

#ifndef CPULOAD_H__INCLUDED
#define CPULOAD_H__INCLUDED

void cpuload_init(void);
void cpuload_read_data(gint type);
void cpuload_draw_1(gint type, GdkPixbuf *pix, GdkColor *bg, GdkColor *fg, GdkColor *err);
void cpuload_draw_all(gint type, GdkPixbuf *pix, GdkColor *bg, GdkColor *fg, GdkColor *err);
void cpuload_discard_data(gint type, gint size);
const gchar *cpuload_tooltip(gint type);

#endif	/* ifndef CPULOAD_H__INCLUDED */
