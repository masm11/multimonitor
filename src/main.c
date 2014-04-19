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

#include "../config.h"
#include <string.h>
#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include "preferences.h"
#include "types.h"
#include "about.h"
#include "battery.h"
#include "cpufreq.h"
#include "loadavg.h"
#include "cpuload.h"
#include "main.h"

static XfcePanelPlugin *plugin;
static GtkWidget *box;
static struct {
    GtkWidget *drawable;
    GdkPixmap *pix;
    PangoLayout *layout;
} work[TYPE_NR];
static GdkGC *bg, *fg, *err;

static struct {
    const char *label;
    void (*read_data)(gint);
    void (*draw_1)(gint, GdkPixmap *, GdkGC *, GdkGC *, GdkGC *);
} funcs[] = {
    { "Battery\nBAT0",   battery_read_data, battery_draw_1 },
    { "Battery\nBAT1",   battery_read_data, battery_draw_1 },
    { "CPU Freq\nCPU 0", cpufreq_read_data, cpufreq_draw_1 },
    { "CPU Freq\nCPU 1", cpufreq_read_data, cpufreq_draw_1 },
    { "CPU Freq\nCPU 2", cpufreq_read_data, cpufreq_draw_1 },
    { "CPU Freq\nCPU 3", cpufreq_read_data, cpufreq_draw_1 },
    { "Loadavg\n1min",   loadavg_read_data, loadavg_draw_1 },
    { "Loadavg\n5min",   loadavg_read_data, loadavg_draw_1 },
    { "Loadavg\n15min",  loadavg_read_data, loadavg_draw_1 },
    { "CPU Load\nCPU 0", cpuload_read_data, cpuload_draw_1 },
    { "CPU Load\nCPU 1", cpuload_read_data, cpuload_draw_1 },
    { "CPU Load\nCPU 2", cpuload_read_data, cpuload_draw_1 },
    { "CPU Load\nCPU 3", cpuload_read_data, cpuload_draw_1 },
};

static gboolean timer(gpointer data)
{
    // データを読む
    for (gint type = 0; type < TYPE_NR; type++)
	(*funcs[type].read_data)(type);
    
    // 1dotずらす
    for (gint type = 0; type < TYPE_NR; type++) {
	gdk_draw_drawable(work[type].pix, fg, work[type].pix,
		1, 0,
		0, 0, work[type].drawable->allocation.width - 1, work[type].drawable->allocation.height);
    }
    
    // 右端の 1dot を描画
    for (gint type = 0; type < TYPE_NR; type++)
	(*funcs[type].draw_1)(type, work[type].pix, bg, fg, err);
    
    // widget にコピー
    for (gint type = 0; type < TYPE_NR; type++) {
	gdk_draw_drawable(work[type].drawable->window, fg, work[type].pix,
		0, 0, 0, 0,
		work[type].drawable->allocation.width, work[type].drawable->allocation.height);
    }
    
    // label を描画
    for (gint type = 0; type < TYPE_NR; type++) {
	gtk_paint_layout(work[type].drawable->style,
		work[type].drawable->window,
		GTK_WIDGET_STATE(work[type].drawable),
		TRUE,
		NULL,	// &event->area,
		work[type].drawable,
		"graph",
		0, 0,
		work[type].layout);
    }

    return TRUE;
}

#if 0
static void save_config_cb(XfcePanelPlugin *plugin, gpointer data)
{
    XfceRc *rc;
    
    gchar *rcfile = xfce_panel_plugin_save_location(plugin, TRUE);
    if (rcfile == NULL)
	return;
    rc = xfce_rc_simple_open(rcfile, FALSE);
    g_free(rcfile);
    if (rc == NULL)
	return;
    
    GList *list = gtk_container_get_children(GTK_CONTAINER(box));
    
    xfce_rc_set_group(rc, NULL);
    xfce_rc_write_int_entry(rc, "num", g_list_length(list));
    
    for (int no = 0; list != NULL; no++) {
	MccGraph *graph = list->data;
	char grp[16];
	
	sprintf(grp, "graph%d", no);
	xfce_rc_set_group(rc, grp);
	
	MccDataSource *src = g_object_get_data(G_OBJECT(graph), "mcc-datasrc");
	
	xfce_rc_write_entry(rc, "datasrc_name", g_type_name_from_instance(&src->object.g_type_instance));
	xfce_rc_write_int_entry(rc, "datasrc_subindex", src->subidx);
	
	for (int i = 0; i < src->nfg; i++) {
	    GdkColor col;
	    mcc_graph_get_fg(graph, i, &col);
	    char key[64], buf[64];
	    sprintf(key, "fg%d", i);
	    sprintf(buf, "#%04x%04x%04x", col.red, col.green, col.blue);
	    xfce_rc_write_entry(rc, key, buf);
	}
	
	for (int i = 0; i < src->nbg; i++) {
	    GdkColor col;
	    mcc_graph_get_bg(graph, i, &col);
	    char key[64], buf[64];
	    sprintf(key, "bg%d", i);
	    sprintf(buf, "#%04x%04x%04x", col.red, col.green, col.blue);
	    xfce_rc_write_entry(rc, key, buf);
	}
	
	{
	    gint width, height;
	    gint size = 0;
	    gtk_widget_get_size_request(GTK_WIDGET(graph), &width, &height);
	    if (width >= 1)
		size = width;
	    if (height >= 1)
		size = height;
	    xfce_rc_write_int_entry(rc, "size", size);
	}
	
	xfce_rc_write_entry(rc, "font", mcc_graph_get_font(graph));
	
	list = g_list_delete_link(list, list);
    }
    
    xfce_rc_close(rc);
}

static void load_config(void)
{
    XfceRc *rc;
    
    gchar *rcfile = xfce_panel_plugin_save_location(plugin, FALSE);
    if (rcfile == NULL)
	return;
    rc = xfce_rc_simple_open(rcfile, TRUE);
    g_free(rcfile);
    if (rc == NULL)
	return;
    
    xfce_rc_set_group(rc, NULL);
    gint num = xfce_rc_read_int_entry(rc, "num", 0);
    
    for (int no = 0; no < num; no++) {
	char grp[16];
	sprintf(grp, "graph%d", no);
	xfce_rc_set_group(rc, grp);
	
	const gchar *srcname = xfce_rc_read_entry(rc, "datasrc_name", NULL);
	gint subidx = xfce_rc_read_int_entry(rc, "datasrc_subindex", -1);
	if (srcname == NULL || subidx < 0) {
	    // これが得られなかったら、どうにもならん。
	    continue;
	}
	
	GType type = g_type_from_name(srcname);
	if (type == 0) {
	    // そんな class は知らねー。
	    continue;
	}
	
	GtkWidget *g = add_graph(type, subidx);
	MccGraph *graph = MCC_GRAPH(g);
	
	MccDataSource *src = g_object_get_data(G_OBJECT(graph), "mcc-datasrc");
	
	for (int i = 0; i < src->nfg; i++) {
	    GdkColor col;
	    gchar key[64];
	    sprintf(key, "fg%d", i);
	    const gchar *cstr = xfce_rc_read_entry(rc, key, NULL);
	    if (cstr != NULL && gdk_color_parse(cstr, &col))
		mcc_graph_set_fg(graph, i, &col);
	}
	
	for (int i = 0; i < src->nbg; i++) {
	    GdkColor col;
	    gchar key[64];
	    sprintf(key, "bg%d", i);
	    const gchar *cstr = xfce_rc_read_entry(rc, key, NULL);
	    if (cstr != NULL && gdk_color_parse(cstr, &col))
		mcc_graph_set_bg(graph, i, &col);
	}
	
	{
	    gint width, height;
	    gint size = xfce_rc_read_int_entry(rc, "size", 0);
	    if (size >= 1) {
		gtk_widget_get_size_request(g, &width, &height);
		if (width >= 1)
		    width = size;
		if (height >= 1)
		    height = size;
		gtk_widget_set_size_request(g, width, height);
	    }
	}
	
	{
	    const gchar *fontname = xfce_rc_read_entry(rc, "font", NULL);
	    if (fontname != NULL)
		mcc_graph_set_font(graph, fontname);
	}
    }
    
    xfce_rc_close(rc);
}

static void print_hier(int indent, GtkWidget *w)
{
    gint width, height;
    gtk_widget_get_size_request(w, &width, &height);
    fprintf(stderr, "%*s%s %dx%d %dx%d %s\n",
	    indent, "",
	    G_OBJECT_TYPE_NAME(w),
	    width, height,
	    w->allocation.width, w->allocation.height,
	    gtk_widget_get_visible(w) ? "visible" : "non-visible"
);
    if (GTK_IS_CONTAINER(w)) {
	GList *list = gtk_container_get_children(GTK_CONTAINER(w));
	for ( ; list != NULL; list = g_list_next(list))
	    print_hier(indent + 1, list->data);
    }
}
#endif

static void change_size_iter(gint type, gint size)
{
    gtk_drawing_area_size(GTK_DRAWING_AREA(work[type].drawable), size, size);
    gtk_widget_queue_resize(work[type].drawable);
    if (work[type].pix != NULL)
	g_object_unref(work[type].pix);
    work[type].pix = gdk_pixmap_new(work[type].drawable->window, size, size, -1);
    gdk_draw_rectangle(work[type].pix, err, TRUE, 0, 0, size, size);
}

static gboolean change_size_cb(GtkWidget *w, gint size, gpointer closure)
{
    if (xfce_panel_plugin_get_orientation(plugin) == GTK_ORIENTATION_VERTICAL)
	gtk_widget_set_size_request(GTK_WIDGET(plugin), size, -1);
    else
	gtk_widget_set_size_request(GTK_WIDGET(plugin), -1, size);
    
    for (gint type = 0; type < TYPE_NR; type++)
	change_size_iter(type, size);
    
    return FALSE;
}

static void change_orient_cb(XfcePanelPlugin *plugin, GtkOrientation orientation, gpointer closure)
{
    GtkWidget *oldbox = box;
    
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
	box = gtk_hbox_new(FALSE, 1);
    } else {
	box = gtk_vbox_new(FALSE, 1);
    }
    gtk_container_set_border_width(GTK_CONTAINER(box), 1);
    
    gtk_widget_show(box);
    
    GList *list = gtk_container_get_children(GTK_CONTAINER(oldbox));
    while (list != NULL) {
	GtkWidget *w = list->data;
	
	g_object_ref(w);
	
	gtk_container_remove(GTK_CONTAINER(oldbox), w);
	
	gint width, height;
	gtk_widget_get_size_request(w, &width, &height);
	if ((orientation == GTK_ORIENTATION_HORIZONTAL && width == -1) ||
		(orientation == GTK_ORIENTATION_VERTICAL && height == -1)) {
	    gint t = width;
	    width = height;
	    height = t;
	}
	gtk_widget_set_size_request(w, width, height);
	
	gtk_box_pack_start(GTK_BOX(box), w, FALSE, FALSE, 0);
	
	g_object_unref(w);
	
	list = g_list_delete_link(list, list);
    }
    
    gtk_widget_destroy(oldbox);
    gtk_container_add(GTK_CONTAINER(plugin), box);
}

#if 0
static void configure_cb(XfcePanelPlugin *plugin, gpointer data)
{
    preferences_create(box, datasrc_types);
    // 戻ってきたら、preferences はもう閉じてる。
    save_config_cb(plugin, NULL);
}
#endif

static void plugin_start(XfcePanelPlugin *plg)
{
    plugin = plg;
    
//    g_signal_connect(plugin, "configure-plugin", G_CALLBACK(configure_cb), NULL);
    g_signal_connect(plugin, "size-changed", G_CALLBACK(change_size_cb), NULL);
    g_signal_connect(plugin, "orientation-changed", G_CALLBACK(change_orient_cb), NULL);
//    g_signal_connect(plugin, "save", G_CALLBACK(save_config_cb), NULL);
    g_signal_connect(plugin, "about", G_CALLBACK(about), NULL);
#if 0
    g_signal_connect(plugin, "free-data", G_CALLBACK(destroy_cb), app);
#endif
    xfce_panel_plugin_menu_show_about(plugin);
    xfce_panel_plugin_menu_show_configure(plugin);
    
    xfce_panel_plugin_set_expand(plugin, TRUE);
    
    if (xfce_panel_plugin_get_orientation(plugin) == GTK_ORIENTATION_HORIZONTAL) {
	box = gtk_hbox_new(FALSE, 0);
    } else {
	box = gtk_vbox_new(FALSE, 0);
    }
    gtk_container_set_border_width(GTK_CONTAINER(box), 1);
    gtk_widget_show(box);
    gtk_container_add(GTK_CONTAINER(plugin), box);
    
    GdkColor color;
    
    color.red = 0;
    color.green = 0;
    color.blue = 0;
    // fixme: gdk_colormap_get_system()
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &color, FALSE, TRUE);
    bg = gdk_gc_new(GTK_WIDGET(plugin)->window);
    gdk_gc_set_foreground(bg, &color);
    
    color.red = 65535;
    color.green = 0;
    color.blue = 0;
    // fixme: gdk_colormap_get_system()
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &color, FALSE, TRUE);
    fg = gdk_gc_new(GTK_WIDGET(plugin)->window);
    gdk_gc_set_foreground(fg, &color);
    
    color.red = 32768;
    color.green = 32768;
    color.blue = 32768;
    // fixme: gdk_colormap_get_system()
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &color, FALSE, TRUE);
    err = gdk_gc_new(GTK_WIDGET(plugin)->window);
    gdk_gc_set_foreground(err, &color);
    
    for (gint type = 0; type < TYPE_NR; type++) {
	work[type].drawable = gtk_drawing_area_new();
	gtk_widget_show(work[type].drawable);
	gtk_box_pack_start(GTK_BOX(box), work[type].drawable, FALSE, FALSE, 0);
    }
    
    PangoFontDescription *font_desc = pango_font_description_from_string("fixed");
    
    for (gint type = 0; type < TYPE_NR; type++) {
	gtk_widget_modify_font(work[type].drawable, font_desc);
	work[type].layout = gtk_widget_create_pango_layout(work[type].drawable, funcs[type].label);
	GdkColor color = {
	    .red = 0xffff,
	    .green = 0xffff,
	    .blue = 0xffff,
	};
	gtk_widget_modify_text(work[type].drawable, GTK_STATE_NORMAL, &color);
    }
    
    pango_font_description_free(font_desc);
    
    //load_config();
    
    g_timeout_add(1000, timer, NULL);
    
    battery_init();
    cpufreq_init();
    loadavg_init();
    cpuload_init();
}

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL(plugin_start)
