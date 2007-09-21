#include <gtk/gtk.h>
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
    { 0, 1 },
    { 0, 2 },
    { 1, 0 },
    { 1, 1 },
    { 2, 0 },
};

#define NR (sizeof idxs / sizeof idxs[0])

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
    gtk_widget_set_size_request(g, 50, 50);
    gtk_widget_show(g);
    gtk_box_pack_start(GTK_BOX(box), g, FALSE, FALSE, 0);
    
    return g;
}

int main(int argc, char **argv)
{
    int i;
    
    gtk_init(&argc, &argv);
    
    for (i = 0; i < NDATASRC; i++)
	(*datasrc_list[i]->sinit)();
    
    GtkWidget *top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    box = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(top), box);
    
    for (i = 0; i < NR; i++) {
	gint idx = idxs[i][0];
	gint subidx = idxs[i][1];
	struct datasrc_t *datasrc = datasrc_list[idx];
	add_graph(datasrc, subidx);
    }
    
    gtk_widget_show_all(top);
    
    g_timeout_add(100, timer, NULL);
    
    preferences_create(box, datasrc_list);
    
    gtk_main();
    
    for (i = 0; i < NDATASRC; i++)
	(*datasrc_list[i]->sfini)();
    
    return 0;
}
