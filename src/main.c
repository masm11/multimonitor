#include <gtk/gtk.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include "mccgraph.h"
#include "mccvalue.h"
#include "datasrc.h"

extern struct datasrc_t *datasrc_list[];

struct datasrc_t *datasrc_list[] = {
    &linux_cpuload_datasrc,
    &linux_cpufreq_datasrc,
    &linux_battery_datasrc,
    NULL,
};

#define NDATASRC (sizeof datasrc_list / sizeof datasrc_list[0] - 1)

static gint idxs[][2] = {
    { 0, 0 },
#if 0
    { 0, 1 },
    { 0, 2 },
    { 1, 0 },
    { 1, 1 },
    { 2, 0 },
#endif
};

#define NR (sizeof idxs / sizeof idxs[0])

static XfcePanelPlugin *plugin;
static GtkWidget *ev;
static GtkWidget *box;

static void update(GtkWidget *widget, gpointer data)
{
    struct datasrc_t *datasrc = g_object_get_data(G_OBJECT(widget), "mcc-datasrc");
    struct datasrc_context_t *ctxt = g_object_get_data(G_OBJECT(widget), "mcc-context");
    
    MccValue *value = (*datasrc->get)(ctxt);
    mcc_graph_add(MCC_GRAPH(widget), value);
}

static gboolean timer(gpointer data)
{
    int i;
    
    for (i = 0; i < NDATASRC; i++)
	(*datasrc_list[i]->sread)();
    
    gtk_container_foreach(GTK_CONTAINER(box), update, NULL);
    
    return TRUE;
}

GtkWidget *add_graph(struct datasrc_t *src, gint subidx)
{
    struct datasrc_context_t *ctxt = (*src->new)(subidx);
    const struct datasrc_context_info_t *ip = (*src->info)(ctxt);
    GtkWidget *g = mcc_graph_new(ip->nvalues, ip->min, ip->max,
	    ip->nfg, ip->default_fg, ip->nbg, ip->default_bg);
    g_object_set_data(G_OBJECT(g), "mcc-datasrc", src);
    g_object_set_data(G_OBJECT(g), "mcc-context", ctxt);
    
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
	
	struct datasrc_t *src = g_object_get_data(G_OBJECT(graph), "mcc-datasrc");
	struct datasrc_context_t *ctxt = g_object_get_data(G_OBJECT(graph), "mcc-context");
	const struct datasrc_context_info_t *info = (*src->info)(ctxt);
	
	gint src_idx = -1;
	for (gint i = 0; i < NDATASRC; i++) {
	    if (datasrc_list[i] == src) {
		src_idx = i;
		break;
	    }
	}
	
	if (src_idx >= 0) {
	    xfce_rc_write_int_entry(rc, "datasrc_index", src_idx);
	    xfce_rc_write_int_entry(rc, "datasrc_subindex", info->sub_idx);
	}
	
	for (int i = 0; i < info->nfg; i++) {
	    GdkColor col;
	    mcc_graph_get_fg(graph, i, &col);
	    char key[64], buf[64];
	    sprintf(key, "fg%d", i);
	    sprintf(buf, "#%04x%04x%04x", col.red, col.green, col.blue);
	    xfce_rc_write_entry(rc, key, buf);
	}
	
	for (int i = 0; i < info->nbg; i++) {
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
	    gtk_widget_get_size_request(graph, &width, &height);
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
    int width, height;
    
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
	box = gtk_hbox_new(FALSE, 1);
	width = 50;
	height = -1;
    } else {
	box = gtk_vbox_new(FALSE, 1);
	width = -1;
	height = 50;
    }
    
    gtk_widget_show(box);
    
    GList *list = gtk_container_get_children(GTK_CONTAINER(oldbox));
    while (list != NULL) {
	GtkWidget *w = list->data;
	g_object_ref(w);
	gtk_container_remove(GTK_CONTAINER(oldbox), w);
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
    preferences_create(box, datasrc_list);
    // 戻ってきたら、preferences はもう閉じてる。
    save_config_cb(plugin, NULL);
}

static void plugin_start(XfcePanelPlugin *plg)
{
    plugin = plg;
    
    for (int i = 0; i < NDATASRC; i++)
	(*datasrc_list[i]->sinit)();
    
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
    gtk_container_add(GTK_CONTAINER(ev), box);
    
    for (int i = 0; i < NR; i++) {
	gint idx = idxs[i][0];
	gint subidx = idxs[i][1];
	struct datasrc_t *datasrc = datasrc_list[idx];
	add_graph(datasrc, subidx);
    }
    
    gtk_widget_show(box);
    
    g_timeout_add(100, timer, NULL);
    
    gtk_main();
    
    for (int i = 0; i < NDATASRC; i++)
	(*datasrc_list[i]->sfini)();
}

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL(plugin_start)
