/* Multi Monitor Plugin for Xfce
 *  Copyright (C) 2007 Yuuki Harano
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

#include <string.h>
#include <math.h>
#include <gtk/gtkmain.h>
#include "mccvalue.h"
#include "mccgraph.h"

typedef struct _MccGraphPrivate {
    gint nvalues;
    
    GList *list;
    guint listlen;
    
    GdkPixmap *pixmap;
    guint16 pix_width, pix_height;
    
    GdkGC *gc_copy;
    GdkGC **gc_fg, **gc_bg;
    GdkGC *gc_bd;
    
    gint nfg;
    GdkColor *fg;
    gint nbg;
    GdkColor *bg;
    
    GdkColor bd;
    
    gdouble min, max;
    gboolean dynamic_scaling;
    gint dynamic_scale;
    
    gchar *fontname;
    gchar *label, *sublabel;
    PangoLayout *layout;
} MccGraphPrivate;

G_DEFINE_TYPE(MccGraph, mcc_graph, GTK_TYPE_MISC)

static void mcc_graph_finalize(GObject *obj);
static void mcc_graph_destroy(GtkObject *object);
static void mcc_graph_realize(GtkWidget *widget);
static void mcc_graph_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static gboolean mcc_graph_expose(GtkWidget *widget, GdkEventExpose *event);
static void create_layout(MccGraph *graph);
static void create_gc(MccGraph *graph);
static void create_pixmap(MccGraph *graph);
static void shift_and_draw(MccGraph *graph);

static inline MccGraphPrivate *mcc_graph_get_private(MccGraph *graph)
{
    return G_TYPE_INSTANCE_GET_PRIVATE(graph, MCC_TYPE_GRAPH, MccGraphPrivate);
}

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
    
    g_type_class_add_private(klass, sizeof (MccGraphPrivate));
}

static void mcc_graph_init(MccGraph *self)
{
    MccGraphPrivate *priv = mcc_graph_get_private(self);
    
    priv->list = NULL;
    priv->listlen = 0;
    
    priv->fg = NULL;
    priv->nbg = 0;
    priv->bg = NULL;
    
    priv->min = 0;
    priv->max = 1.0;
    
    priv->bd.red = 0xffff;
    priv->bd.green = 0xffff;
    priv->bd.blue = 0xffff;
    
    priv->label = g_strdup("label");
    priv->sublabel = g_strdup("sublabel");
}

static void mcc_graph_finalize(GObject *object)
{
    MccGraph *graph = MCC_GRAPH(object);
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    
    while (priv->list != NULL) {
	MccValue *v = priv->list->data;
	priv->list = g_list_delete_link(priv->list, priv->list);
	mcc_value_unref(v);
    }
    
#define UNREF(p)  do { if (p != NULL) { g_object_unref(p); p = NULL; } } while (FALSE)
#define FREE(p) do { if (p != NULL) { g_free(p); p = NULL; } } while (FALSE)
    UNREF(priv->pixmap);
    UNREF(priv->gc_copy);
    
    if (priv->gc_fg != NULL) {
	for (gint i = 0; i < priv->nfg; i++)
	    UNREF(priv->gc_fg[i]);
	FREE(priv->gc_fg);
    }
    if (priv->gc_bg != NULL) {
	for (gint i = 0; i < priv->nbg; i++)
	    UNREF(priv->gc_bg[i]);
	FREE(priv->gc_bg);
    }
    UNREF(priv->gc_bd);
    
    FREE(priv->fg);
    FREE(priv->bg);
    
    FREE(priv->fontname);
    FREE(priv->label);
    FREE(priv->sublabel);
    
    UNREF(priv->layout);
    
    (*G_OBJECT_CLASS(mcc_graph_parent_class)->finalize)(object);
}

static void mcc_graph_destroy(GtkObject *object)
{
    (*GTK_OBJECT_CLASS(mcc_graph_parent_class)->destroy)(object);
}

static void mcc_graph_realize(GtkWidget *widget)
{
    MccGraph *graph = MCC_GRAPH(widget);
    
    (*GTK_WIDGET_CLASS(mcc_graph_parent_class)->realize)(widget);
    
    create_layout(graph);
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
	MccGraphPrivate *priv = mcc_graph_get_private(graph);

	gdk_draw_drawable(widget->window, priv->gc_copy, priv->pixmap,
		0, 0,
		widget->allocation.width - priv->pix_width,
		widget->allocation.height - priv->pix_height,
		priv->pix_width, priv->pix_height);
	
	gtk_paint_layout(widget->style,
		widget->window,
		GTK_WIDGET_STATE(widget),
		TRUE,
		&event->area,
		widget,
		"graph",
		0, 0,
		priv->layout);
	
	return TRUE;
    }
    return FALSE;
}

static gint calc_dynamic_scale(MccGraph *graph)
{
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    
    if (!priv->dynamic_scaling)
	return 1;
    
    gdouble max_v = 1.0;
    gint x;
    GList *lp;
    for (lp = priv->list, x = priv->pix_width - 1; lp != NULL && x >= 0; lp = lp->next, x--) {
	MccValue *value = lp->data;
	gdouble v = 0;
	for (gint i = 0; i < priv->nvalues; i++)
	    v += mcc_value_get_value(value, i);
	if (max_v < v)
	    max_v = v;
    }
    
    gint scale = (gint) ceil((max_v - priv->min) / (priv->max - priv->min));
    if (scale < 1)
	scale = 1;
    
    return scale;
}

static void create_layout(MccGraph *graph)
{
    GtkWidget *widget = GTK_WIDGET(graph);
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    
    PangoFontDescription *font_desc = pango_font_description_from_string(priv->fontname);
    gtk_widget_modify_font(widget, font_desc);
    pango_font_description_free(font_desc);
    
    if (priv->layout != NULL)
	g_object_unref(priv->layout);
    gchar *str = g_strdup_printf("%s\n%s", priv->label, priv->sublabel);
    priv->layout = gtk_widget_create_pango_layout(widget, str);
    g_free(str);
    
    GdkColor color = {
	.red = 0xffff,
	.green = 0xffff,
	.blue = 0xffff,
    };
    gtk_widget_modify_text(widget, GTK_STATE_NORMAL, &color);
}

static void create_gc(MccGraph *graph)
{
    GtkWidget *widget = &graph->misc.widget;
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    
    GdkColormap *cmap = gdk_colormap_get_system();
    
    if (priv->gc_copy != NULL)
	g_object_unref(priv->gc_copy);
    if (priv->gc_fg != NULL) {
	for (gint i = 0; i < priv->nfg; i++) {
	    if (priv->gc_fg[i] != NULL) {
		g_object_unref(priv->gc_fg[i]);
		priv->gc_fg[i] = NULL;
	    }
	}
	g_free(priv->gc_fg);
	priv->gc_fg = NULL;
    }
    if (priv->gc_bg != NULL) {
	for (gint i = 0; i < priv->nbg; i++) {
	    if (priv->gc_bg[i] != NULL) {
		g_object_unref(priv->gc_bg[i]);
		priv->gc_bg[i] = NULL;
	    }
	}
	g_free(priv->gc_bg);
	priv->gc_bg = NULL;
    }
    
    priv->gc_copy = gdk_gc_new(widget->window);		// fixme: no_window にしたいな。
    gdk_gc_set_function(priv->gc_copy, GDK_COPY);
    
    priv->gc_bg = g_new0(GdkGC *, priv->nbg);
    for (gint i = 0; i < priv->nbg; i++) {
	priv->gc_bg[i] = gdk_gc_new(widget->window);
	gdk_gc_set_function(priv->gc_bg[i], GDK_COPY);
	GdkColor col = priv->bg[i];
	gdk_colormap_alloc_color(cmap, &col, FALSE, TRUE);
	gdk_gc_set_foreground(priv->gc_bg[i], &col);
    }
    
    priv->gc_fg = g_new0(GdkGC *, priv->nfg);
    for (gint i = 0; i < priv->nfg; i++) {
	priv->gc_fg[i] = gdk_gc_new(widget->window);
	gdk_gc_set_function(priv->gc_fg[i], GDK_COPY);
	GdkColor col = priv->fg[i];
	gdk_colormap_alloc_color(cmap, &col, FALSE, TRUE);
	gdk_gc_set_foreground(priv->gc_fg[i], &col);
    }
    
    {
	priv->gc_bd = gdk_gc_new(widget->window);
	gdk_gc_set_function(priv->gc_bd, GDK_COPY);
	GdkColor col = priv->bd;
	gdk_colormap_alloc_color(cmap, &col, FALSE, TRUE);
	gdk_gc_set_foreground(priv->gc_bd, &col);
    }
}

static void create_pixmap(MccGraph *graph)
{
    GtkWidget *widget = &graph->misc.widget;
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    
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
    
    gint x;
    GList *lp;
    for (lp = priv->list, x = priv->pix_width - 1; lp != NULL; lp = lp->next, x--) {
	MccValue *value = lp->data;
	gint col_bg = mcc_value_get_background(value);
	gdk_draw_line(priv->pixmap, priv->gc_bg[col_bg], x, 0, x, priv->pix_height);
	for (gint i = priv->nvalues - 1; i >= 0; i--) {
	    gdouble v = 0;
	    for (gint j = 0; j <= i; j++)
		v += mcc_value_get_value(value, j);
	    if (priv->dynamic_scaling)
		v /= priv->dynamic_scale;
	    gint col_fg = mcc_value_get_foreground(value, i);
	    gint h = (v - priv->min) * priv->pix_height / (priv->max - priv->min);
	    if (h > 0) {
		gdk_draw_line(priv->pixmap, priv->gc_fg[col_fg],
			x, priv->pix_height - h, x, priv->pix_height);
	    }
	}
    }
    
    for ( ; x >= 0; x--)
	gdk_draw_line(priv->pixmap, priv->gc_bg[0], x, 0, x, priv->pix_height);
    
    if (priv->dynamic_scaling) {
	for (gint n = 1; n < priv->dynamic_scale; n++) {
	    gint h = n * priv->pix_height / priv->dynamic_scale;
	    gdk_draw_line(priv->pixmap, priv->gc_bd,
		    0, priv->pix_height - h, priv->pix_width, priv->pix_height - h);
	}
    }
    
    gtk_widget_queue_clear(GTK_WIDGET(graph));
}

static void shift_and_draw(MccGraph *graph)
{
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    if (priv->pixmap != NULL) {
	gdk_draw_drawable(priv->pixmap, priv->gc_copy, priv->pixmap,
		1, 0, 0, 0, priv->pix_width - 1, priv->pix_height);
	
	guint x = priv->pix_width - 1;
	MccValue *value = priv->list->data;
	gint col_bg = mcc_value_get_background(value);
	gdk_draw_line(priv->pixmap, priv->gc_bg[col_bg], x, 0, x, priv->pix_height);
	for (gint i = priv->nvalues - 1; i >= 0; i--) {
	    gdouble v = 0;
	    for (gint j = 0; j <= i; j++)
		v += mcc_value_get_value(value, j);
	    if (priv->dynamic_scaling)
		v /= priv->dynamic_scale;
	    gint col_fg = mcc_value_get_foreground(value, i);
	    gint h = (v - priv->min) * priv->pix_height / (priv->max - priv->min);
	    if (h > 0) {
		gdk_draw_line(priv->pixmap, priv->gc_fg[col_fg],
			x, priv->pix_height - h, x, priv->pix_height);
	    }
	}
	
	if (priv->dynamic_scaling) {
	    for (gint n = 1; n < priv->dynamic_scale; n++) {
		gint h = n * priv->pix_height / priv->dynamic_scale;
		gdk_draw_line(priv->pixmap, priv->gc_bd,
			priv->pix_width - 1, priv->pix_height - h, priv->pix_width - 1, priv->pix_height - h);
	    }
	}
    }
    
    gtk_widget_queue_clear(GTK_WIDGET(graph));
}

void mcc_graph_add(MccGraph *graph, MccValue *value)
{
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    
    mcc_value_ref(value);
    
    priv->list = g_list_prepend(priv->list, value);
    
    if (++priv->listlen > priv->pix_width) {
	GList *last = g_list_nth(priv->list, priv->pix_width - 1);
	if (last != NULL) {
	    GList *lp;
	    while ((lp = g_list_next(last)) != NULL) {
		MccValue *v = lp->data;
		priv->list = g_list_delete_link(priv->list, lp);
		mcc_value_unref(v);
		priv->listlen--;
	    }
	}
    }
    
    gboolean recreate = FALSE;
    if (priv->dynamic_scaling) {
	gint scale = calc_dynamic_scale(graph);
	if (priv->dynamic_scale != scale) {
	    priv->dynamic_scale = scale;
	    recreate = TRUE;
	}
    }
    
    if (!recreate)
	shift_and_draw(graph);
    else
	create_pixmap(graph);
}

void mcc_graph_get_fg(MccGraph *graph, gint i, GdkColor *fg)
{
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    g_return_if_fail((guint) i < priv->nfg);
    *fg = priv->fg[i];
}

void mcc_graph_get_bg(MccGraph *graph, gint i, GdkColor *bg)
{
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    g_return_if_fail((guint) i < priv->nbg);
    *bg = priv->bg[i];
}

void mcc_graph_set_fg(MccGraph *graph, gint i, const GdkColor *fg)
{
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    g_return_if_fail((guint) i < priv->nfg);
    priv->fg[i] = *fg;
    create_gc(graph);
    create_pixmap(graph);
}

void mcc_graph_set_bg(MccGraph *graph, gint i, const GdkColor *bg)
{
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    g_return_if_fail((guint) i < priv->nbg);
    priv->bg[i] = *bg;
    create_gc(graph);
    create_pixmap(graph);
}

void mcc_graph_set_font(MccGraph *graph, const gchar *fontname)
{
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    if (priv->fontname != NULL)
	g_free(priv->fontname);
    priv->fontname = g_strdup(fontname);
    create_layout(graph);
}

const gchar *mcc_graph_get_font(MccGraph *graph)
{
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    return priv->fontname;
}

GtkWidget *mcc_graph_new(gint nvalues, gdouble min, gdouble max,
	gint nfg, const GdkColor *fg,
	gint nbg, const GdkColor *bg,
	gboolean dynamic_scaling,
	const gchar *label, const gchar *sublabel)
{
    MccGraph *graph;
    
    graph = g_object_new(MCC_TYPE_GRAPH, NULL);
    MccGraphPrivate *priv = mcc_graph_get_private(graph);
    priv->nvalues = nvalues;
    priv->min = min;
    priv->max = max;
    priv->nfg = nfg;
    priv->fg = g_new0(GdkColor, nfg);
    priv->nbg = nbg;
    priv->bg = g_new0(GdkColor, nbg);
    priv->dynamic_scaling = dynamic_scaling;
    
    memcpy(priv->fg, fg, sizeof(GdkColor) * nfg);
    memcpy(priv->bg, bg, sizeof(GdkColor) * nbg);
    
    priv->fontname = g_strdup("sans 6");
    priv->label = g_strdup(label);
    priv->sublabel = g_strdup(sublabel);
    
    return GTK_WIDGET(graph);
}

/*EOF*/
