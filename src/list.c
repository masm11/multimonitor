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

#include <glib.h>
#include "list.h"

GList *list_truncate(GList *list, gint len)
{
    GList *lp = g_list_nth(list, len);	// 最初の element は 0。
    if (lp == NULL)
	return list;	// list がそんなに長くなかった。
    
    if (lp->prev == NULL) {	// len=0。
	g_list_free_full(list, g_free);
	return NULL;
    }
    
    lp->prev->next = NULL;
    lp->prev = NULL;
    g_list_free_full(lp, g_free);
    return list;
}
