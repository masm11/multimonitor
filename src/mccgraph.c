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
    GdkPixmap *pixmap;
    guint16 pix_width, pix_height;
    
    GdkGC *gc_copy;
    GdkGC *gc_fg, *gc_bg;
} MccGraphPrivate;

G_DEFINE_TYPE(MccGraph, mcc_graph, GTK_TYPE_MISC)

static void mcc_graph_finalize(GObject *obj);
static void mcc_graph_destroy(GtkObject *object);
static void mcc_graph_realize(GtkWidget *widget);
static gboolean mcc_graph_expose(GtkWidget *widget, GdkEventExpose *event);
static void create_pixmap(MccGraph *graph);
static void shift_and_draw(MccGraph *graph);

static void mcc_graph_class_init(MccGraphClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkObjectClass *object_class = GTK_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    
    gobject_class->finalize = mcc_graph_finalize;
    
    object_class->destroy = mcc_graph_destroy;
    
    widget_class->expose_event = mcc_graph_expose;
    widget_class->realize = mcc_graph_realize;
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

static void mcc_graph_realize(GtkWidget *widget)
{
    MccGraph *graph = MCC_GRAPH(widget);
    MccGraphPrivate *priv = graph->priv;
    
    (*GTK_WIDGET_CLASS(mcc_graph_parent_class)->realize)(widget);
    
    GdkColormap *cmap = gdk_colormap_get_system();
    
    priv->gc_copy = gdk_gc_new(widget->window);
    priv->gc_fg = gdk_gc_new(widget->window);
    priv->gc_bg = gdk_gc_new(widget->window);
    
    gdk_gc_set_function(priv->gc_copy, GDK_COPY);
    gdk_gc_set_function(priv->gc_fg, GDK_COPY);
    gdk_gc_set_function(priv->gc_bg, GDK_COPY);
    
    GdkColor fg, bg;
    gdk_color_parse("#000000", &bg);
    gdk_color_parse("#ffffff", &fg);
    
    gdk_colormap_alloc_color(cmap, &bg, FALSE, TRUE);
    gdk_colormap_alloc_color(cmap, &fg, FALSE, TRUE);
    
    gdk_gc_set_foreground(priv->gc_bg, &bg);
    gdk_gc_set_foreground(priv->gc_fg, &fg);
    
    create_pixmap(graph);
}

static gboolean mcc_graph_expose(GtkWidget *widget, GdkEventExpose *event)
{
    MccGraph *graph = MCC_GRAPH(widget);
    
    if (GTK_WIDGET_VISIBLE(widget) && GTK_WIDGET_MAPPED(widget)) {
	gdk_draw_drawable(widget->window, graph->priv->gc_copy, graph->priv->pixmap,
		0, 0,
		widget->allocation.width - graph->priv->pix_width,
		widget->allocation.height - graph->priv->pix_height,
		graph->priv->pix_width, graph->priv->pix_height);
	
	return TRUE;
    }
    return FALSE;
}

static void create_pixmap(MccGraph *graph)
{
    GtkWidget *widget = &graph->misc.widget;
    MccGraphPrivate *priv = graph->priv;
    
    if (priv->pixmap != NULL) {
	if (priv->pix_width != widget->allocation.width || priv->pix_height != widget->allocation.height) {
	    g_object_unref(priv->pixmap);
	    priv->pixmap = NULL;
	}
    }
    
    if (priv->pixmap == NULL) {
	priv->pixmap = gdk_pixmap_new(widget->window,
		widget->allocation.width, widget->allocation.height,
		gdk_drawable_get_depth(widget->window));
	priv->pix_width = widget->allocation.width;
	priv->pix_height = widget->allocation.height;
    }
    
    gdk_draw_rectangle(priv->pixmap, priv->gc_bg, TRUE, 0, 0, widget->allocation.width, widget->allocation.height);
    
    gint x;
    GList *lp;
    for (lp = graph->priv->list, x = widget->allocation.width - 1; lp != NULL; lp = lp->next, x--) {
	gint v = GPOINTER_TO_INT(lp->data);
	gdk_draw_line(priv->pixmap, priv->gc_fg, x, v, x, widget->allocation.height);
    }
}

static void shift_and_draw(MccGraph *graph)
{
    MccGraphPrivate *priv = graph->priv;
    if (priv->pixmap != NULL) {
	gdk_draw_drawable(priv->pixmap, priv->gc_copy, priv->pixmap,
		1, 0, 0, 0, priv->pix_width - 1, priv->pix_height);
	
	guint x = priv->pix_width - 1;
	gdk_draw_line(priv->pixmap, priv->gc_bg, x, 0, x, priv->pix_height);
	gint v = GPOINTER_TO_INT(priv->list->data);
	gdk_draw_line(priv->pixmap, priv->gc_fg, x, v, x, priv->pix_height);
    }
}

void mcc_graph_add(MccGraph *graph, gint val)
{
    graph->priv->list = g_list_prepend(graph->priv->list, GINT_TO_POINTER(val));
    shift_and_draw(graph);
    gtk_widget_queue_clear(GTK_WIDGET(graph));
}

GtkWidget *mcc_graph_new(void)
{
    MccGraph *graph;
    
    graph = g_object_new(MCC_TYPE_GRAPH, NULL);
    
    return GTK_WIDGET(graph);
}

/*EOF*/
