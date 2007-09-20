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
    COL_PAGE,
    COL_DATASRC,
    COL_DATASRC_CTXT,
    COL_NR,
};

struct work_t {
    GtkWidget *dialog;
    GtkWidget *curr_page;
    GtkWidget *page_box;
    GtkListStore *store;
};

static GtkWidget *create_page(struct datasrc_t *src, struct datasrc_context_t *ctxt)
{
    const struct datasrc_info_t *sip = (*src->sinfo)();
    const struct datasrc_context_info_t *ip = (*src->info)(ctxt);
    gchar buf[128];
    
    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    GtkWidget *w, *frm, *tbl;
    
    sprintf(buf, "%s - %s", sip->label, ip->sublabel);
    w = gtk_label_new(buf);
    gtk_misc_set_alignment(GTK_MISC(w), 0, 0.5);
    gtk_widget_show(w);
    gtk_box_pack_start(GTK_BOX(vbox), w, FALSE, FALSE, 0);
    
    frm = gtk_frame_new("Foreground");
    gtk_widget_show(frm);
    gtk_box_pack_start(GTK_BOX(vbox), frm, FALSE, FALSE, 0);
    
    tbl = gtk_table_new(ip->nfg, 2, FALSE);
    gtk_widget_show(tbl);
    gtk_container_add(GTK_CONTAINER(frm), tbl);
    for (int i = 0; i < ip->nfg; i++) {
	w = gtk_label_new(ip->fg_labels[i]);
	gtk_misc_set_alignment(GTK_MISC(w), 1.0, 0.5);
	gtk_widget_show(w);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 0, 1, i, i + 1);
	
	w = gtk_color_button_new_with_color(&ip->default_fg[i]);
	gtk_widget_show(w);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 1, 2, i, i + 1);
    }
    
    frm = gtk_frame_new("Background");
    gtk_widget_show(frm);
    gtk_box_pack_start(GTK_BOX(vbox), frm, FALSE, FALSE, 0);
    
    tbl = gtk_table_new(ip->nfg, 2, FALSE);
    gtk_widget_show(tbl);
    gtk_container_add(GTK_CONTAINER(frm), tbl);
    for (int i = 0; i < ip->nbg; i++) {
	w = gtk_label_new(ip->bg_labels[i]);
	gtk_widget_show(w);
	gtk_misc_set_alignment(GTK_MISC(w), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 0, 1, i, i + 1);
	
	w = gtk_color_button_new_with_color(&ip->default_bg[i]);
	gtk_widget_show(w);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 1, 2, i, i + 1);
    }
    
    return vbox;
}

static void selected(GtkTreeSelection *sel, gpointer user_data)
{
    struct work_t *w = user_data;
    GtkTreeView *tree = gtk_tree_selection_get_tree_view(sel);
    GtkTreeModel *model;
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
	GtkWidget *page;
	gtk_tree_model_get(model, &iter, COL_PAGE, &page, -1);
	
	if (w->curr_page != page) {
	    if (w->curr_page != NULL)
		gtk_widget_hide(w->curr_page);
	    w->curr_page = page;
	    gtk_widget_show(page);
	}
    }
}

static void add_page(GtkWidget *widget, gpointer data)
{
    struct work_t *w = data;
    GtkTreeIter iter;
    
    struct datasrc_t *datasrc = g_object_get_data(G_OBJECT(widget), "mcc-datasrc");
    struct datasrc_context_t *ctxt = g_object_get_data(G_OBJECT(widget), "mcc-context");
    
    GtkWidget *page = create_page(datasrc, ctxt);
    
    const struct datasrc_info_t *sip = (*datasrc->sinfo)();
    const struct datasrc_context_info_t *ip = (*datasrc->info)(ctxt);
    gchar buf[128];
    sprintf(buf, "%s - %s", sip->label, ip->sublabel);
    
    gtk_list_store_append(w->store, &iter);
    gtk_box_pack_start(GTK_BOX(w->page_box), page, FALSE, FALSE, 0);
    gtk_list_store_set(w->store, &iter,
	    COL_LABEL, buf,
	    COL_PAGE, page,
	    COL_DATASRC, datasrc,
	    COL_DATASRC_CTXT, ctxt,
	    -1);
}

void preferences_create(GtkWidget *graph_box, const struct datasrc_t * const *datasrcs)
{
    struct work_t work, *w = &work;
    
    memset(w, 0, sizeof *w);
    
    w->dialog = gtk_dialog_new_with_buttons(
	    "Multi Monitor Preferences", NULL, GTK_DIALOG_MODAL,
	    "Close", GTK_RESPONSE_CLOSE,
	    NULL);
    
    w->page_box = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(w->page_box);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(w->dialog)->vbox), w->page_box, FALSE, FALSE, 0);
    
    w->store = gtk_list_store_new(COL_NR, G_TYPE_STRING, GTK_TYPE_WIDGET, G_TYPE_POINTER, G_TYPE_POINTER);
    GtkWidget *tree;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(w->store));
    gtk_widget_show(tree);

    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_BROWSE);
    g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(selected), &work);
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Test", renderer,
	    "text", COL_LABEL,
	    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
    gtk_box_pack_start(GTK_BOX(w->page_box), tree, FALSE, FALSE, 0);
    
    {
	GtkTreeIter iter;
	gtk_list_store_append(w->store, &iter);
	GtkWidget *page = preferences_new_create(graph_box, datasrcs);
	gtk_box_pack_start(GTK_BOX(w->page_box), page, FALSE, FALSE, 0);
	gtk_list_store_set(w->store, &iter,
		COL_LABEL, "New",
		COL_PAGE, page,
		COL_DATASRC_CTXT, NULL,
		-1);
    }
    
    gtk_container_foreach(GTK_CONTAINER(graph_box), add_page, &work);
    
    gtk_dialog_run(GTK_DIALOG(w->dialog));
    
    gtk_widget_destroy(w->dialog);
}
