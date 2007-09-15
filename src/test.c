#include <gtk/gtk.h>
#include "mccgraph.h"
#include "mccvalue.h"
#include "ops.h"

static struct ops_t *ops_list[] = {
    &linux_cpuload_ops,
};

#define NOPS (sizeof ops_list / sizeof ops_list[0])

static gint idxs[] = {
    0, 0, 0,
};

#define NR (sizeof idxs / sizeof idxs[0])

static GtkWidget *box;

static void update(GtkWidget *widget, gpointer data)
{
    struct ops_t *ops = g_object_get_data(G_OBJECT(widget), "mcc-ops");
    void *ptr = g_object_get_data(G_OBJECT(widget), "mcc-ptr");
    
    MccValue *value = (*ops->get)(ptr);
    mcc_graph_add(MCC_GRAPH(widget), value);
}

static gboolean timer(gpointer data)
{
    int i;
    
    for (i = 0; i < NOPS; i++)
	(*ops_list[i]->sread)();
    
    gtk_container_foreach(GTK_CONTAINER(box), update, NULL);
    
    return TRUE;
}

int main(int argc, char **argv)
{
    int i;
    
    gtk_init(&argc, &argv);
    
    for (i = 0; i < NOPS; i++)
	(*ops_list[i]->sinit)();
    
    GtkWidget *top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    box = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(top), box);
    
    for (i = 0; i < NR; i++) {
	gint idx = idxs[i];
	struct ops_t *ops = ops_list[idx];
	void *ptr = (*ops->new)();
	GtkWidget *g = mcc_graph_new((*ops->nvalues)(ptr));
	g_object_set_data(G_OBJECT(g), "mcc-ops", ops);
	g_object_set_data(G_OBJECT(g), "mcc-ptr", ptr);
	gtk_widget_set_size_request(g, 50, 50);
	gtk_box_pack_start(GTK_BOX(box), g, FALSE, FALSE, 0);
    }
    
    gtk_widget_show_all(top);
    
    g_timeout_add(100, timer, NULL);
    
    gtk_main();
    
    for (i = 0; i < NOPS; i++)
	(*ops_list[i]->sfini)();
    
    return 0;
}
