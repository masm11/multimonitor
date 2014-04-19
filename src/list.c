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
