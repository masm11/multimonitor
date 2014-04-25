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

#ifndef DISK_H__INCLUDED
#define DISK_H__INCLUDED

void disk_init(void);
void disk_read_data(gint type);
void disk_draw_1(gint type, GdkPixbuf *pix);
void disk_draw_all(gint type, GdkPixbuf *pix);
void disk_discard_data(gint type, gint size);
const gchar *disk_tooltip(gint type);

#endif	/* ifndef DISK_H__INCLUDED */
