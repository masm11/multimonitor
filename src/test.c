#include <gtk/gtk.h>
#include "mccgraph.h"
#include "mccvalue.h"
#include "datasrc.h"

static struct datasrc_t *datasrc_list[] = {
    &linux_cpuload_datasrc,
    &linux_cpufreq_datasrc,
};

#define NDATASRC (sizeof datasrc_list / sizeof datasrc_list[0])

static gint idxs[] = {
    0, 0, 0, 1, 1,
};

#define NR (sizeof idxs / sizeof idxs[0])

static GtkWidget *box;

static void update(GtkWidget *widget, gpointer data)
{
    struct datasrc_t *datasrc = g_object_get_data(G_OBJECT(widget), "mcc-datasrc");
    void *ptr = g_object_get_data(G_OBJECT(widget), "mcc-ptr");
    
    MccValue *value = (*datasrc->get)(ptr);
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
	gint idx = idxs[i];
	struct datasrc_t *datasrc = datasrc_list[idx];
	void *ptr = (*datasrc->new)();
	const struct datasrc_info_t *ip = (*datasrc->info)(ptr);
	GtkWidget *g = mcc_graph_new(ip->nvalues, ip->min, ip->max);
	g_object_set_data(G_OBJECT(g), "mcc-datasrc", datasrc);
	g_object_set_data(G_OBJECT(g), "mcc-ptr", ptr);
	gtk_widget_set_size_request(g, 50, 50);
	gtk_box_pack_start(GTK_BOX(box), g, FALSE, FALSE, 0);
    }
    
    gtk_widget_show_all(top);
    
    g_timeout_add(100, timer, NULL);
    
    gtk_main();
    
    for (i = 0; i < NDATASRC; i++)
	(*datasrc_list[i]->sfini)();
    
    return 0;
}
