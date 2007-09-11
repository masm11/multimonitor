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

#include "mccvalue.h"
#include "mccgraph.h"

typedef struct _MccGraphPrivate {
    GList *list;
    guint nvalues;
    
    GdkPixmap *pixmap;
    guint16 pix_width, pix_height;
    
    GdkGC *gc_copy;
    GdkGC *gc_fg, *gc_bg;
    
    gint min, max;
} MccGraphPrivate;

G_DEFINE_TYPE(MccGraph, mcc_graph, GTK_TYPE_MISC)

static void mcc_graph_finalize(GObject *obj);
static void mcc_graph_destroy(GtkObject *object);
static void mcc_graph_realize(GtkWidget *widget);
static void mcc_graph_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static gboolean mcc_graph_expose(GtkWidget *widget, GdkEventExpose *event);
static void create_gc(MccGraph *graph);
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
    widget_class->size_allocate = mcc_graph_size_allocate;
}

static void mcc_graph_init(MccGraph *self)
{
    self->priv = g_new0(struct _MccGraphPrivate, 1);
    
    self->priv->list = NULL;
    self->priv->nvalues = 0;
    
    self->priv->min = 0;
    self->priv->max = 5;
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
    
    create_gc(graph);
    create_pixmap(graph);
}

static void mcc_graph_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
    MccGraph *graph = MCC_GRAPH(widget);
    
    (*GTK_WIDGET_CLASS(mcc_graph_parent_class)->size_allocate)(widget, allocation);
    
    if (GTK_WIDGET_VISIBLE(widget) && GTK_WIDGET_MAPPED(widget)) {
	create_pixmap(graph);
    }
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

static void create_gc(MccGraph *graph)
{
    GtkWidget *widget = &graph->misc.widget;
    MccGraphPrivate *priv = graph->priv;
    
    GdkColormap *cmap = gdk_colormap_get_system();
    
    if (priv->gc_copy != NULL)
	g_object_unref(priv->gc_copy);
    if (priv->gc_fg != NULL)
	g_object_unref(priv->gc_fg);
    if (priv->gc_bg != NULL)
	g_object_unref(priv->gc_bg);
    
    priv->gc_copy = gdk_gc_new(widget->window);		// fixme: no_window にしたいな。
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
    
    gdk_draw_rectangle(priv->pixmap, priv->gc_bg, TRUE, 0, 0, priv->pix_width, priv->pix_height);
    
    gint x;
    GList *lp;
    for (lp = graph->priv->list, x = priv->pix_width - 1; lp != NULL; lp = lp->next, x--) {
	MccValue *value = lp->data;
	gdouble v = mcc_value_get_value(0, value);
	gint h = (v - priv->min) * priv->pix_height / (priv->max - priv->min);
	if (h > 0) {
	    gdk_draw_line(priv->pixmap, priv->gc_fg,
		    x, priv->pix_height - h, x, priv->pix_height);
	}
    }
    
    gtk_widget_queue_clear(GTK_WIDGET(graph));
}

static void shift_and_draw(MccGraph *graph)
{
    MccGraphPrivate *priv = graph->priv;
    if (priv->pixmap != NULL) {
	gdk_draw_drawable(priv->pixmap, priv->gc_copy, priv->pixmap,
		1, 0, 0, 0, priv->pix_width - 1, priv->pix_height);
	
	guint x = priv->pix_width - 1;
	gdk_draw_line(priv->pixmap, priv->gc_bg, x, 0, x, priv->pix_height);
	MccValue *value = priv->list->data;
	gint v = mcc_value_get_value(0, value);
	gint h = (v - priv->min) * priv->pix_height / (priv->max - priv->min);
	if (h > 0) {
	    gdk_draw_line(priv->pixmap, priv->gc_fg,
		    x, priv->pix_height - h, x, priv->pix_height);
	}
    }
    
    gtk_widget_queue_clear(GTK_WIDGET(graph));
}

void mcc_graph_add(MccGraph *graph, MccValue *value)
{
    g_object_ref(value);
    graph->priv->list = g_list_prepend(graph->priv->list, value);
    
    if (++graph->priv->nvalues > graph->priv->pix_width) {
	GList *last = g_list_nth(graph->priv->list, graph->priv->pix_width - 1);
	if (last != NULL) {
	    GList *lp;
	    while ((lp = g_list_next(last)) != NULL) {
		MccValue *v = lp->data;
		graph->priv->list = g_list_delete_link(graph->priv->list, lp);
		g_object_unref(v);
		graph->priv->nvalues--;
	    }
	}
    }
    
    shift_and_draw(graph);
}

GtkWidget *mcc_graph_new(void)
{
    MccGraph *graph;
    
    graph = g_object_new(MCC_TYPE_GRAPH, NULL);
    
    return GTK_WIDGET(graph);
}

/*EOF*/
