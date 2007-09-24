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
#include "mccgraph.h"
#include "mccdatasource.h"

enum {
    NEW_COL_LABEL,
    NEW_COL_TYPE,
    NEW_COL_SUB_IDX,
    NEW_COL_NR,
};

struct new_work_t {
    struct list_work_t *ww;
    GtkWidget *vbox;
    GtkWidget *btn;
    GtkWidget *graph_box;
    GType type;
    gint subidx;
};

static void list_add_graph(struct list_work_t *w, GType type, gint subidx);

static void new_selected(GtkTreeSelection *sel, gpointer user_data)
{
    struct new_work_t *w = user_data;
    GtkTreeModel *model;
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
	GType type;
	gint subidx;
	gtk_tree_model_get(model, &iter,
		NEW_COL_TYPE, &type,
		NEW_COL_SUB_IDX, &subidx,
		-1);
	w->type = type;
	w->subidx = subidx;
	gtk_widget_set_sensitive(w->btn, type != 0);
    }
}

static void new_clicked(GtkWidget *widget, gpointer userdata)
{
    struct new_work_t *w = userdata;
    if (w->type != 0)
	list_add_graph(w->ww, w->type, w->subidx);
}

static GtkWidget *new_create(struct list_work_t *ww, GtkWidget *graph_box, GType *datasrc_types)
{
    struct new_work_t *w = g_new0(struct new_work_t, 1);
    
    w->ww = ww;
    w->graph_box = graph_box;
    
    GtkTreeStore *store = gtk_tree_store_new(NEW_COL_NR, G_TYPE_STRING, G_TYPE_ULONG, G_TYPE_INT);	// fixme: ulong?
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    w->vbox = gtk_vbox_new(FALSE, 0);
    
    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_widget_show(tree);
    gtk_box_pack_start(GTK_BOX(w->vbox), tree, FALSE, FALSE, 0);
    
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_BROWSE);
    g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(new_selected), w);
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Test", renderer,
	    "text", NEW_COL_LABEL,
	    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
    
    for (GType *types = datasrc_types; *types != 0; types++) {
	GType type = *types;
	MccDataSourceClass *class = g_type_class_peek(type);
	GtkTreeIter iter0;
	gtk_tree_store_append(store, &iter0, NULL);
	gtk_tree_store_set(store, &iter0,
		NEW_COL_LABEL, class->label,
		NEW_COL_TYPE, 0,
		NEW_COL_SUB_IDX, 0,
		-1);
	
	for (gint subidx = 0; class->sublabels[subidx] != NULL; subidx++) {
	    GtkTreeIter iter1;
	    gtk_tree_store_append(store, &iter1, &iter0);
	    gtk_tree_store_set(store, &iter1,
		    NEW_COL_LABEL, class->sublabels[subidx],
		    NEW_COL_TYPE, type,
		    NEW_COL_SUB_IDX, subidx,
		    -1);
	}
    }
    
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);
    gtk_box_pack_end(GTK_BOX(w->vbox), hbox, FALSE, FALSE, 0);
    
    w->btn = gtk_button_new_with_label("Add");
    g_signal_connect(w->btn, "clicked", G_CALLBACK(new_clicked), w);
    gtk_widget_show(w->btn);
    gtk_box_pack_end(GTK_BOX(hbox), w->btn, FALSE, FALSE, 0);
    
    {
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter))
	    gtk_tree_selection_select_iter(sel, &iter);
    }
    
    return w->vbox;
}



enum {
    LST_COL_LABEL,
    LST_COL_PAGE,
    LST_COL_DATASRC,
    LST_COL_NR,
};

struct list_work_t {
    GtkWidget *dialog;
    GtkWidget *curr_page;
    GtkWidget *page_box;
    GtkListStore *store;
};

static void color_changed(GtkWidget *widget, gpointer userdata)
{
    MccGraph *graph = g_object_get_data(G_OBJECT(widget), "mcc-pref-graph");
    const gchar *type = g_object_get_data(G_OBJECT(widget), "mcc-pref-color-type");
    gint index = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "mcc-pref-color-index"));
    GdkColor color;
    
    gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), &color);
    
    if (strcmp(type, "fg") == 0)
	mcc_graph_set_fg(graph, index, &color);
    if (strcmp(type, "bg") == 0)
	mcc_graph_set_bg(graph, index, &color);
}

static void size_changed(GtkSpinButton *spinbutton, gpointer userdata)
{
    MccGraph *graph = userdata;
    gint width, height;
    gtk_widget_get_size_request(GTK_WIDGET(graph), &width, &height);
    
    gint size = gtk_spin_button_get_value_as_int(spinbutton);
    if (width >= 1)
	width = size;
    if (height >= 1)
	height = size;
    gtk_widget_set_size_request(GTK_WIDGET(graph), width, height);
}

static void delete_cb(GtkButton *button, gpointer userdata)
{
    struct list_work_t *w = userdata;
    GtkWidget *graph = g_object_get_data(G_OBJECT(button), "mcc-pref-graph");
    GtkWidget *page = g_object_get_data(G_OBJECT(button), "mcc-pref-page");
    
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(w->store), &iter)) {
	while (TRUE) {
	    GtkWidget *p;
	    gtk_tree_model_get(GTK_TREE_MODEL(w->store), &iter, LST_COL_PAGE, &p, -1);
	    if (p == page) {
		gtk_list_store_remove(w->store, &iter);
		break;
	    }
	    gtk_tree_model_iter_next(GTK_TREE_MODEL(w->store), &iter);
	}
    }
    
    gtk_widget_destroy(page);
    gtk_widget_destroy(graph);
}

static GtkWidget *list_create_page(
	MccDataSource *src, MccGraph *graph, struct list_work_t *lw)
{
    MccDataSourceClass *class = MCC_DATA_SOURCE_GET_CLASS(src);
    gchar buf[128];
    
    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    GtkWidget *w, *frm, *tbl;
    
    sprintf(buf, "%s - %s", class->label, src->sublabel);
    w = gtk_label_new(buf);
    gtk_misc_set_alignment(GTK_MISC(w), 0, 0.5);
    gtk_widget_show(w);
    gtk_box_pack_start(GTK_BOX(vbox), w, FALSE, FALSE, 0);
    
    frm = gtk_frame_new("Foreground");
    gtk_widget_show(frm);
    gtk_box_pack_start(GTK_BOX(vbox), frm, FALSE, FALSE, 0);
    
    tbl = gtk_table_new(src->nfg, 2, FALSE);
    gtk_widget_show(tbl);
    gtk_container_add(GTK_CONTAINER(frm), tbl);
    for (int i = 0; i < src->nfg; i++) {
	w = gtk_label_new(src->fg_labels[i]);
	gtk_misc_set_alignment(GTK_MISC(w), 1.0, 0.5);
	gtk_widget_show(w);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 0, 1, i, i + 1);
	
	GdkColor fg;
	mcc_graph_get_fg(MCC_GRAPH(graph), i, &fg);
	w = gtk_color_button_new_with_color(&fg);
	g_object_set_data(G_OBJECT(w), "mcc-pref-graph", graph);
	g_object_set_data(G_OBJECT(w), "mcc-pref-color-type", "fg");
	g_object_set_data(G_OBJECT(w), "mcc-pref-color-index", GINT_TO_POINTER(i));
	g_signal_connect(G_OBJECT(w), "color-set", G_CALLBACK(color_changed), NULL);
	gtk_widget_show(w);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 1, 2, i, i + 1);
    }
    
    frm = gtk_frame_new("Background");
    gtk_widget_show(frm);
    gtk_box_pack_start(GTK_BOX(vbox), frm, FALSE, FALSE, 0);
    
    tbl = gtk_table_new(src->nfg, 2, FALSE);
    gtk_widget_show(tbl);
    gtk_container_add(GTK_CONTAINER(frm), tbl);
    for (int i = 0; i < src->nbg; i++) {
	w = gtk_label_new(src->bg_labels[i]);
	gtk_widget_show(w);
	gtk_misc_set_alignment(GTK_MISC(w), 1.0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 0, 1, i, i + 1);
	
	GdkColor bg;
	mcc_graph_get_bg(MCC_GRAPH(graph), i, &bg);
	w = gtk_color_button_new_with_color(&bg);
	g_object_set_data(G_OBJECT(w), "mcc-pref-graph", graph);
	g_object_set_data(G_OBJECT(w), "mcc-pref-color-type", "bg");
	g_object_set_data(G_OBJECT(w), "mcc-pref-color-index", GINT_TO_POINTER(i));
	g_signal_connect(G_OBJECT(w), "color-set", G_CALLBACK(color_changed), NULL);
	gtk_widget_show(w);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 1, 2, i, i + 1);
    }
    
    {
	gint width, height;
	gtk_widget_get_size_request(GTK_WIDGET(graph), &width, &height);
	
	tbl = gtk_table_new(1, 2, FALSE);
	gtk_widget_show(tbl);
	gtk_box_pack_start(GTK_BOX(vbox), tbl, FALSE, FALSE, 0);
	
	w = gtk_label_new("Size");
	gtk_widget_show(w);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 0, 1, 0, 1);
	
	w = gtk_spin_button_new_with_range(1, 1000, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), width == -1 ? height : width);
	g_signal_connect(w, "value-changed", G_CALLBACK(size_changed), graph);
	gtk_widget_show(w);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 1, 2, 0, 1);
    }
    
    {
	GtkWidget *w = gtk_button_new_with_label("Delete");
	gtk_box_pack_start(GTK_BOX(vbox), w, FALSE, FALSE, 0);
	g_object_set_data(G_OBJECT(w), "mcc-pref-graph", graph);
	g_object_set_data(G_OBJECT(w), "mcc-pref-page", vbox);
	g_signal_connect(w, "clicked", G_CALLBACK(delete_cb), lw);
	gtk_widget_show(w);
    }
    
    return vbox;
}

static void list_selected(GtkTreeSelection *sel, gpointer user_data)
{
    struct list_work_t *w = user_data;
    GtkTreeModel *model;
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
	GtkWidget *page;
	gtk_tree_model_get(model, &iter, LST_COL_PAGE, &page, -1);
	
	if (w->curr_page != page) {
	    if (w->curr_page != NULL)
		gtk_widget_hide(w->curr_page);
	    w->curr_page = page;
	    gtk_widget_show(page);
	}
    }
}

static void list_add_page(GtkWidget *widget, gpointer data)
{
    struct list_work_t *w = data;
    GtkTreeIter iter;
    
    MccDataSource *src = g_object_get_data(G_OBJECT(widget), "mcc-datasrc");
    MccDataSourceClass *class = MCC_DATA_SOURCE_GET_CLASS(src);
    
    GtkWidget *page = list_create_page(src, MCC_GRAPH(widget), w);
    
    gchar buf[128];
    sprintf(buf, "%s - %s", class->label, src->sublabel);
    
    gtk_list_store_append(w->store, &iter);
    gtk_box_pack_start(GTK_BOX(w->page_box), page, FALSE, FALSE, 0);
    gtk_list_store_set(w->store, &iter,
	    LST_COL_LABEL, buf,
	    LST_COL_PAGE, page,
	    LST_COL_DATASRC, src,
	    -1);
}

static void list_add_graph(struct list_work_t *w, GType type, gint subidx)
{
    GtkWidget *g = add_graph(type, subidx);
    list_add_page(g, w);
}

void preferences_create(GtkWidget *graph_box, GType *datasrc_types)
{
    struct list_work_t work, *w = &work;
    
    memset(w, 0, sizeof *w);
    
    w->dialog = gtk_dialog_new_with_buttons(
	    "Multi Monitor Preferences", NULL, GTK_DIALOG_MODAL,
	    "Close", GTK_RESPONSE_CLOSE,
	    NULL);
    
    w->page_box = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(w->page_box);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(w->dialog)->vbox), w->page_box, FALSE, FALSE, 0);
    
    w->store = gtk_list_store_new(LST_COL_NR, G_TYPE_STRING, GTK_TYPE_WIDGET, G_TYPE_POINTER);
    GtkWidget *tree;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(w->store));
    gtk_widget_show(tree);
    
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_BROWSE);
    g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(list_selected), &work);
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Test", renderer,
	    "text", LST_COL_LABEL,
	    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
    gtk_box_pack_start(GTK_BOX(w->page_box), tree, FALSE, FALSE, 0);
    
    {
	GtkTreeIter iter;
	gtk_list_store_append(w->store, &iter);
	GtkWidget *page = new_create(w, graph_box, datasrc_types);
	gtk_box_pack_start(GTK_BOX(w->page_box), page, FALSE, FALSE, 0);
	gtk_list_store_set(w->store, &iter,
		LST_COL_LABEL, "New",
		LST_COL_PAGE, page,
		-1);
    }
    
    gtk_container_foreach(GTK_CONTAINER(graph_box), list_add_page, &work);
    
    gtk_dialog_run(GTK_DIALOG(w->dialog));
    
    gtk_widget_destroy(w->dialog);
}
