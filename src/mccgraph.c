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

#include "mccgraph.h"

typedef struct _MccGraphPrivate {
    GList *list;
} MccGraphPrivate;

G_DEFINE_TYPE(MccGraph, mcc_graph, GTK_TYPE_MISC)

static void mcc_graph_finalize(GObject *obj);
static void mcc_graph_destroy(GtkObject *object);
static gboolean mcc_graph_expose(GtkWidget *widget, GdkEventExpose *event);

static void mcc_graph_class_init(MccGraphClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkObjectClass *object_class = GTK_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    
    gobject_class->finalize = mcc_graph_finalize;
    
    object_class->destroy = mcc_graph_destroy;
    
    widget_class->expose_event = mcc_graph_expose;
    
}

static void mcc_graph_init(MccGraph *self)
{
    self->priv = g_new0(struct _MccGraphPrivate, 1);
    
    self->priv->list = NULL;
}

static void mcc_graph_finalize(GObject *object)
{
    (*G_OBJECT_CLASS(mcc_graph_parent_class)->finalize)(object);
}

static void mcc_graph_destroy(GtkObject *object)
{
    (*GTK_OBJECT_CLASS(mcc_graph_parent_class)->destroy)(object);
}

static gboolean mcc_graph_expose(GtkWidget *widget, GdkEventExpose *event)
{
    MccGraph *graph = MCC_GRAPH(widget);
    
    if (GTK_WIDGET_VISIBLE(widget) && GTK_WIDGET_MAPPED(widget)) {
	guint width = widget->allocation.width;
	guint height = widget->allocation.height;
	
	printf("%dx%d.\n", width, height);
	
	GdkColormap *cmap = gdk_colormap_get_system();
	
	GdkColor fg, bg;
	gdk_color_parse("#000000", &bg);
	gdk_color_parse("#ffffff", &fg);
	
	gdk_colormap_alloc_color(cmap, &bg, FALSE, TRUE);
	gdk_colormap_alloc_color(cmap, &fg, FALSE, TRUE);
	
	GdkGC *gc = gdk_gc_new(widget->window);
	gdk_gc_set_function(gc, GDK_COPY);
	
	gdk_gc_set_foreground(gc, &bg);
	gdk_draw_rectangle(widget->window, gc, TRUE, 0, 0, width, height);
	
	gdk_gc_set_foreground(gc, &fg);
	
	gint x;
	GList *lp;
	for (lp = graph->priv->list, x = width - 1; lp != NULL; lp = lp->next, x--) {
	    gint v = GPOINTER_TO_INT(lp->data);
	    gdk_draw_line(widget->window, gc, x, v, x, height);
	}
	
	g_object_unref(gc);
	return TRUE;
    }
    return FALSE;
}

void mcc_graph_add(MccGraph *graph, gint val)
{
    graph->priv->list = g_list_prepend(graph->priv->list, GINT_TO_POINTER(val));
    gtk_widget_queue_clear(GTK_WIDGET(graph));
}

GtkWidget *mcc_graph_new(void)
{
    MccGraph *graph;
    
    graph = g_object_new(MCC_TYPE_GRAPH, NULL);
    
    return GTK_WIDGET(graph);
}

/*EOF*/
