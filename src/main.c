#include <string.h>
#include <gtk/gtk.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include "mccgraph.h"
#include "mccvalue.h"
#include "mccdatasource.h"
#include "mccsrccpufreq.h"
#include "mccsrccpuload.h"
#include "mccsrcbattery.h"
#include "mccsrcloadavg.h"

static GType *datasrc_types;

static XfcePanelPlugin *plugin;
static GtkWidget *ev;
static GtkWidget *box;

static void update(GtkWidget *widget, gpointer data)
{
    MccDataSource *src = g_object_get_data(G_OBJECT(widget), "mcc-datasrc");
    
    if (src->add_on_tick || mcc_data_source_has_new_data(src)) {
	MccValue *value = mcc_data_source_get(src);
	mcc_graph_add(MCC_GRAPH(widget), value);
    }
}

static gboolean timer(gpointer data)
{
    for (gint i = 0; datasrc_types[i] != 0; i++)
	mcc_data_source_read(datasrc_types[i]);
    
    gtk_container_foreach(GTK_CONTAINER(box), update, NULL);
    
    return TRUE;
}

GtkWidget *add_graph(GType type, gint subidx)
{
    MccDataSource *src = mcc_data_source_new(type, subidx);
    GtkWidget *g = mcc_graph_new(src->nvalues, src->min, src->max,
	    src->nfg, src->default_fg, src->nbg, src->default_bg, src->dynamic_scaling);
    g_object_set_data_full(G_OBJECT(g), "mcc-datasrc", src, g_object_unref);
    
    int width, height;
    if (xfce_panel_plugin_get_orientation(plugin) == GTK_ORIENTATION_HORIZONTAL) {
	width = 50;
	height = -1;
    } else {
	width = -1;
	height = 50;
    }

    gtk_widget_set_size_request(g, width, height);
    gtk_widget_show(g);
    gtk_box_pack_start(GTK_BOX(box), g, FALSE, FALSE, 0);
    
    return g;
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
    
    GList *list = gtk_container_get_children(GTK_CONTAINER(box));
    
    xfce_rc_set_group(rc, NULL);
    xfce_rc_write_int_entry(rc, "num", g_list_length(list));
    
    for (int no = 0; list != NULL; no++) {
	MccGraph *graph = list->data;
	char grp[16];
	
	sprintf(grp, "graph%d", no);
	xfce_rc_set_group(rc, grp);
	
	MccDataSource *src = g_object_get_data(G_OBJECT(graph), "mcc-datasrc");
	GType type = src->object.g_type_instance.g_class->g_type;	// fixme: ... :-(
	
	gint src_idx = -1;
	for (gint i = 0; datasrc_types[i] != 0; i++) {
	    if (datasrc_types[i] == type) {
		src_idx = i;
		break;
	    }
	}
	
	if (src_idx >= 0) {
	    xfce_rc_write_int_entry(rc, "datasrc_index", src_idx);
	    xfce_rc_write_int_entry(rc, "datasrc_subindex", src->subidx);
	}
	
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
	
	list = g_list_next(list);		// fixme: 解放
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
	
	gint idx = xfce_rc_read_int_entry(rc, "datasrc_index", -1);
	gint subidx = xfce_rc_read_int_entry(rc, "datasrc_subindex", -1);
	if (idx < 0 || subidx < 0) {
	    // これが得られなかったら、どうにもならん。
	    continue;
	}
	
	GtkWidget *g = add_graph(datasrc_types[idx], subidx);
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
    }
    
    xfce_rc_close(rc);
}

static gboolean change_size_cb(GtkWidget *w, gint size, gpointer closure)
{
#if 0
    if (get_applet_vert(plugin))
	gtk_widget_set_size_request(GTK_WIDGET(plugin), size, -1);
    else
#endif
	
    // gtk_container_foreach(GTK_CONTAINER(app->pack), change_size_iter, NULL);
	
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
	
	list = g_list_next(list);	// fixme: free
    }
    
    gtk_widget_destroy(oldbox);
    gtk_container_add(GTK_CONTAINER(ev), box);
}

static void configure_cb(XfcePanelPlugin *plugin, gpointer data)
{
    preferences_create(box, datasrc_types);
    // 戻ってきたら、preferences はもう閉じてる。
    save_config_cb(plugin, NULL);
}

static void plugin_start(XfcePanelPlugin *plg)
{
    plugin = plg;
    
    GType types[] = {
	MCC_TYPE_SRC_CPU_FREQ,
	MCC_TYPE_SRC_CPU_LOAD,
	MCC_TYPE_SRC_BATTERY,
	MCC_TYPE_SRC_LOAD_AVG,
    };
    datasrc_types = g_new0(GType, sizeof types / sizeof types[0] + 1);
    memcpy(datasrc_types, types, sizeof types);
    
    // fixme: class の初期化のつもり。
    for (int i = 0; datasrc_types[i] != 0; i++) {
	g_type_class_ref(datasrc_types[i]);
    }
    
    g_signal_connect(plugin, "configure-plugin", G_CALLBACK(configure_cb), NULL);
    g_signal_connect(plugin, "size-changed", G_CALLBACK(change_size_cb), NULL);
    g_signal_connect(plugin, "orientation-changed", G_CALLBACK(change_orient_cb), NULL);
    g_signal_connect(plugin, "save", G_CALLBACK(save_config_cb), NULL);
#if 0
    g_signal_connect(plugin, "free-data", G_CALLBACK(destroy_cb), app);
    g_signal_connect(plugin, "about", G_CALLBACK(about), app);
    
    xfce_panel_plugin_menu_show_about(plugin);
#endif
    xfce_panel_plugin_menu_show_configure(plugin);
    
    ev = gtk_event_box_new();
    gtk_widget_show(ev);
    xfce_panel_plugin_add_action_widget(plugin, ev);
    gtk_container_add(GTK_CONTAINER(plugin), ev);
    
    if (xfce_panel_plugin_get_orientation(plugin) == GTK_ORIENTATION_HORIZONTAL) {
	box = gtk_hbox_new(FALSE, 1);
    } else {
	box = gtk_vbox_new(FALSE, 1);
    }
    gtk_widget_show(box);
    gtk_container_add(GTK_CONTAINER(ev), box);
    
    load_config();
    
    GList *list = gtk_container_get_children(GTK_CONTAINER(box));
    if (list == NULL) {
	GType type = datasrc_types[0];
	add_graph(type, 0);
    }
    
    g_timeout_add(100, timer, NULL);
    
    gtk_main();
    
#if 0
    for (int i = 0; datasrc_list[i] != NULL; i++)
	(*datasrc_list[i]->sfini)();
#endif
}

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL(plugin_start)
