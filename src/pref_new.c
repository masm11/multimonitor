/* 
 *  Copyright (C) 2007 Yuuki Harano
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include <gtk/gtk.h>
#include <string.h>
#include "datasrc.h"

enum {
    COL_LABEL,
    COL_DATASRC,
    COL_SUB_IDX,
    COL_NR,
};

struct work_t {
    GtkWidget *vbox;
    GtkWidget *btn;
    GtkWidget *graph_box;
    struct datasrc_t *src;
    gint subidx;
};

static void selected(GtkTreeSelection *sel, gpointer user_data)
{
    struct work_t *w = user_data;
    GtkTreeView *tree = gtk_tree_selection_get_tree_view(sel);
    GtkTreeModel *model;
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
	struct datasrc_t *src;
	gint subidx;
	gtk_tree_model_get(model, &iter,
		COL_DATASRC, &src,
		COL_SUB_IDX, &subidx,
		-1);
	w->src = src;
	w->subidx = subidx;
	gtk_widget_set_sensitive(w->btn, w->src != NULL);
    }
}

static void clicked(GtkWidget *widget, gpointer userdata)
{
    struct work_t *w = userdata;
    if (w->src != NULL)
	add_graph(w->src, w->subidx);
}

GtkWidget *preferences_new_create(GtkWidget *graph_box, const struct datasrc_t * const *srcs)
{
    struct work_t *w = g_new0(struct work_t, 1);
    
    w->graph_box = graph_box;
    
    GtkTreeStore *store = gtk_tree_store_new(COL_NR, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT);
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    w->vbox = gtk_vbox_new(FALSE, 0);
    
    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_widget_show(tree);
    gtk_box_pack_start(GTK_BOX(w->vbox), tree, FALSE, FALSE, 0);
    
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_BROWSE);
    g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(selected), w);
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Test", renderer,
	    "text", COL_LABEL,
	    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
    
    for (const struct datasrc_t * const *ds = srcs; *ds != NULL; ds++) {
	const struct datasrc_info_t *sip = (*(*ds)->sinfo)();
	GtkTreeIter iter0;
	gtk_tree_store_append(store, &iter0, NULL);
	gtk_tree_store_set(store, &iter0,
		COL_LABEL, sip->label,
		COL_DATASRC, NULL,
		COL_SUB_IDX, 0,
		-1);
	
	for (gint subidx = 0; sip->sublabels[subidx] != NULL; subidx++) {
	    GtkTreeIter iter1;
	    gtk_tree_store_append(store, &iter1, &iter0);
	    gtk_tree_store_set(store, &iter1,
		    COL_LABEL, sip->sublabels[subidx],
		    COL_DATASRC, *ds,
		    COL_SUB_IDX, subidx,
		    -1);
	}
    }
    
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);
    gtk_box_pack_end(GTK_BOX(w->vbox), hbox, FALSE, FALSE, 0);
    
    w->btn = gtk_button_new_with_label("Add");
    g_signal_connect(w->btn, "clicked", G_CALLBACK(clicked), w);
    gtk_widget_show(w->btn);
    gtk_box_pack_end(GTK_BOX(hbox), w->btn, FALSE, FALSE, 0);
    
    return w->vbox;
}
