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
#include <cairo.h>
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
    GtkWidget *ev;
    GtkWidget *drawable;
    GdkPixbuf *pix;
    cairo_t *cr;
    PangoLayout *layout;
    gboolean show;
} work[TYPE_NR];
static GdkColor bg, fg, err;
static const char *fontname = "sans 8";

static struct {
    const char *label, *sublabel;
    void (*read_data)(gint);
    void (*draw_1)(gint, GdkPixbuf *, GdkColor *, GdkColor *, GdkColor *);
    void (*discard_data)(gint, gint);
} funcs[] = {
    { "Battery",  "BAT0",  battery_read_data, battery_draw_1, battery_discard_data },
    { "Battery",  "BAT1",  battery_read_data, battery_draw_1, battery_discard_data },
    { "CPU Freq", "CPU 0", cpufreq_read_data, cpufreq_draw_1, cpufreq_discard_data },
    { "CPU Freq", "CPU 1", cpufreq_read_data, cpufreq_draw_1, cpufreq_discard_data },
    { "CPU Freq", "CPU 2", cpufreq_read_data, cpufreq_draw_1, cpufreq_discard_data },
    { "CPU Freq", "CPU 3", cpufreq_read_data, cpufreq_draw_1, cpufreq_discard_data },
    { "Loadavg",  "1min",  loadavg_read_data, loadavg_draw_1, loadavg_discard_data },
    { "Loadavg",  "5min",  loadavg_read_data, loadavg_draw_1, loadavg_discard_data },
    { "Loadavg",  "15min", loadavg_read_data, loadavg_draw_1, loadavg_discard_data },
    { "CPU Load", "CPU 0", cpuload_read_data, cpuload_draw_1, cpuload_discard_data },
    { "CPU Load", "CPU 1", cpuload_read_data, cpuload_draw_1, cpuload_discard_data },
    { "CPU Load", "CPU 2", cpuload_read_data, cpuload_draw_1, cpuload_discard_data },
    { "CPU Load", "CPU 3", cpuload_read_data, cpuload_draw_1, cpuload_discard_data },
};

#if 0
static void print_hier(GtkWidget *w, gint indent)
{
    fprintf(stderr, "%*s%s %dx%d+%d+%d %s window\n",
	    indent, "", G_OBJECT_TYPE_NAME(w),
	    w->allocation.width, w->allocation.height,
	    w->allocation.x, w->allocation.y,
	    gtk_widget_get_has_window(w) ? "has" : "no");
    if (GTK_IS_CONTAINER(w)) {
	GList *list = gtk_container_get_children(GTK_CONTAINER(w));
	for ( ; list != NULL; list = g_list_next(list))
	    print_hier(GTK_WIDGET(list->data), indent + 2);
    }
}
#endif

static gboolean timer(gpointer data)
{
    // データを読む
    for (gint type = 0; type < TYPE_NR; type++) {
	(*funcs[type].read_data)(type);
    }
    
    // 1dotずらす
    for (gint type = 0; type < TYPE_NR; type++) {
	guchar *pixels = gdk_pixbuf_get_pixels(work[type].pix);
	gint rowstride = gdk_pixbuf_get_rowstride(work[type].pix);
	gint nch = gdk_pixbuf_get_n_channels(work[type].pix);
	guchar *dst = pixels;
	guchar *src = dst + nch;
	guint len = nch * (gdk_pixbuf_get_width(work[type].pix) - 1);
	for (gint y = 0; y < gdk_pixbuf_get_height(work[type].pix); y++) {
	    memmove(dst, src, len);
	    src += rowstride;
	    dst += rowstride;
	}
    }
    
    // 右端の 1dot を描画
    for (gint type = 0; type < TYPE_NR; type++)
	(*funcs[type].draw_1)(type, work[type].pix, &bg, &fg, &err);
    
    // widget にコピー
    for (gint type = 0; type < TYPE_NR; type++) {
	if (!work[type].show)
	    continue;
	if (work[type].drawable->window == NULL)
	    continue;
	
	cairo_t *dst = gdk_cairo_create(work[type].drawable->window);
	gdk_cairo_set_source_pixbuf(dst, work[type].pix, 0, 0);
	cairo_rectangle(dst, 0, 0, work[type].drawable->allocation.width, work[type].drawable->allocation.height);
	cairo_fill(dst);
	cairo_destroy(dst);
    }
    
    // label を描画
    for (gint type = 0; type < TYPE_NR; type++) {
	if (!work[type].show)
	    continue;
	
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
    
    // 余分なデータを捨てる。
    for (gint type = 0; type < TYPE_NR; type++)
	(*funcs[type].discard_data)(type, work[type].drawable->allocation.width);
    
    // print_hier(GTK_WIDGET(plugin), 0);
    
    return TRUE;
}

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
    
    for (gint type = 0; type < TYPE_NR; type++) {
	char key[128];
	snprintf(key, sizeof key, "show%d", type);
	xfce_rc_write_bool_entry(rc, key, work[type].show);
    }
    
    xfce_rc_write_entry(rc, "fontname", fontname);
    
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
    
    for (gint type = 0; type < TYPE_NR; type++) {
	char key[128];
	snprintf(key, sizeof key, "show%d", type);
	work[type].show = xfce_rc_read_bool_entry(rc, key, TRUE);
    }
    
    fontname = g_strdup(xfce_rc_read_entry(rc, "fontname", "sans 8"));
    
    xfce_rc_close(rc);
}

static void change_size_iter(gint type, gint size)
{
    gtk_drawing_area_size(GTK_DRAWING_AREA(work[type].drawable), size, size);
    gtk_widget_queue_resize(work[type].drawable);
    
    if (work[type].pix != NULL)
	g_object_unref(work[type].pix);
    work[type].pix = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, size, size);
    
    guchar *pixels = gdk_pixbuf_get_pixels(work[type].pix);
    gint rowstride = gdk_pixbuf_get_rowstride(work[type].pix);
    memset(pixels, 0x80, rowstride * size);
}

static gboolean change_size_cb(GtkWidget *w, gint size, gpointer closure)
{
    for (gint type = 0; type < TYPE_NR; type++)
	change_size_iter(type, size);
    
    return TRUE;
}

static void change_orient_cb(XfcePanelPlugin *plugin, GtkOrientation orientation, gpointer closure)
{
    GtkWidget *oldbox = box;
    
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
	box = gtk_hbox_new(FALSE, 1);
    } else {
	box = gtk_vbox_new(FALSE, 1);
    }
    gtk_container_set_border_width(GTK_CONTAINER(box), 0);
    
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

static void configure_toggled_cb(GtkWidget *btn, gpointer data)
{
    gint type = GPOINTER_TO_SIZE(data);
    work[type].show = !work[type].show;
    if (work[type].show)
	gtk_widget_show(work[type].ev);
    else
	gtk_widget_hide(work[type].ev);
}

static void font_set_cb(GtkWidget *btn, gpointer data)
{
    fontname = g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(btn)));
    PangoFontDescription *font_desc = pango_font_description_from_string(fontname);
    
    for (gint type = 0; type < TYPE_NR; type++) {
	gtk_widget_modify_font(work[type].drawable, font_desc);
	char label[128];
	snprintf(label, sizeof label, "%s\n%s", funcs[type].label, funcs[type].sublabel);
	work[type].layout = gtk_widget_create_pango_layout(work[type].drawable, label);
	GdkColor color = {
	    .red = 0xffff,
	    .green = 0xffff,
	    .blue = 0xffff,
	};
	gtk_widget_modify_text(work[type].drawable, GTK_STATE_NORMAL, &color);
    }
    
    pango_font_description_free(font_desc);
}

static void configure_cb(XfcePanelPlugin *plugin, gpointer data)
{
    GtkWidget *dialog;
    
    dialog = gtk_dialog_new_with_buttons(
	    "Multi Monitor Preferences", NULL, GTK_DIALOG_MODAL,
	    "Close", GTK_RESPONSE_CLOSE,
	    NULL);
    gtk_widget_show(dialog);
    
    GtkWidget *frame = gtk_frame_new("Show or Hide");
    gtk_widget_show(frame);
    gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame, FALSE, FALSE, 0);
    
    GtkWidget *box = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(box);
    gtk_container_set_border_width(GTK_CONTAINER(box), 5);
    gtk_container_add(GTK_CONTAINER(frame), box);
    
    for (gint type = 0; type < TYPE_NR; type++) {
	char label[128];
	snprintf(label, sizeof label, "%s %s", funcs[type].label, funcs[type].sublabel);
	GtkWidget *btn = gtk_check_button_new_with_label(label);
	gtk_widget_show(btn);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(btn), work[type].show);
	g_signal_connect(btn, "toggled", G_CALLBACK(configure_toggled_cb), GSIZE_TO_POINTER(type));
	gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 0);
    }
    
    GtkWidget *fframe = gtk_frame_new("Font");
    gtk_widget_show(fframe);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), fframe, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(fframe), 5);
    
    GtkWidget *fbtn = gtk_font_button_new();
    gtk_widget_show(fbtn);
    gtk_container_add(GTK_CONTAINER(fframe), fbtn);
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(fbtn), fontname);
    g_signal_connect(G_OBJECT(fbtn), "font-set", G_CALLBACK(font_set_cb), NULL);
    
    GtkWidget *sep = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), sep, FALSE, FALSE, 0);
    gtk_widget_show(sep);
    
    gtk_dialog_run(GTK_DIALOG(dialog));
    
    gtk_widget_destroy(dialog);
    
    save_config_cb(plugin, NULL);
}

static void plugin_start(XfcePanelPlugin *plg)
{
    plugin = plg;
    
    g_signal_connect(plugin, "configure-plugin", G_CALLBACK(configure_cb), NULL);
    g_signal_connect(plugin, "size-changed", G_CALLBACK(change_size_cb), NULL);
    g_signal_connect(plugin, "orientation-changed", G_CALLBACK(change_orient_cb), NULL);
    g_signal_connect(plugin, "save", G_CALLBACK(save_config_cb), NULL);
    g_signal_connect(plugin, "about", G_CALLBACK(about), NULL);
#if 0
    g_signal_connect(plugin, "free-data", G_CALLBACK(destroy_cb), app);
#endif
    xfce_panel_plugin_menu_show_about(plugin);
    xfce_panel_plugin_menu_show_configure(plugin);
    
    if (xfce_panel_plugin_get_orientation(plugin) == GTK_ORIENTATION_HORIZONTAL) {
	box = gtk_hbox_new(FALSE, 1);
    } else {
	box = gtk_vbox_new(FALSE, 1);
    }
    gtk_container_set_border_width(GTK_CONTAINER(box), 0);
    gtk_widget_show(box);
    gtk_container_add(GTK_CONTAINER(plugin), box);
    xfce_panel_plugin_add_action_widget(plugin, box);
    
    bg = (GdkColor) { 0, 0, 0, 0 };
    fg = (GdkColor) { 0, 65535, 0, 0 };
    err = (GdkColor) { 0, 32768, 32768, 32768 };
    
    for (gint type = 0; type < TYPE_NR; type++) {
	work[type].ev = gtk_event_box_new();
	gtk_widget_show(work[type].ev);
	gtk_box_pack_start(GTK_BOX(box), work[type].ev, FALSE, FALSE, 0);
	xfce_panel_plugin_add_action_widget(plugin, work[type].ev);
	
	work[type].drawable = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(work[type].drawable), 40, 40);
	gtk_widget_show(work[type].drawable);
	gtk_container_add(GTK_CONTAINER(work[type].ev), work[type].drawable);
	work[type].show = TRUE;
	
	work[type].pix = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 40, 40);
    }
    
    load_config();
    
    PangoFontDescription *font_desc = pango_font_description_from_string(fontname);
    
    for (gint type = 0; type < TYPE_NR; type++) {
	gtk_widget_modify_font(work[type].drawable, font_desc);
	char label[128];
	snprintf(label, sizeof label, "%s\n%s", funcs[type].label, funcs[type].sublabel);
	work[type].layout = gtk_widget_create_pango_layout(work[type].drawable, label);
	GdkColor color = {
	    .red = 0xffff,
	    .green = 0xffff,
	    .blue = 0xffff,
	};
	gtk_widget_modify_text(work[type].drawable, GTK_STATE_NORMAL, &color);
    }
    
    pango_font_description_free(font_desc);
    
    for (gint type = 0; type < TYPE_NR; type++) {
	if (work[type].show)
	    gtk_widget_show(work[type].ev);
	else
	    gtk_widget_hide(work[type].ev);
    }
    
    g_timeout_add(1000, timer, NULL);
    
    battery_init();
    cpufreq_init();
    loadavg_init();
    cpuload_init();
}

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL(plugin_start)
