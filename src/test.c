#include <gtk/gtk.h>
#include "mccgraph.h"
#include "mccvalue.h"
#include "ops.h"

static struct ops_t *ops[] = {
    &linux_cpuload_ops,
};

#define NR (sizeof ops / sizeof ops[0])

static void *ptrs[NR];
static GtkWidget *graph[NR];

static gboolean timer(gpointer data)
{
    int i;
    
    for (i = 0; i < NR; i++)
	(*ops[i]->sread)();
    
    for (i = 0; i < NR; i++) {
	MccValue *value = (*ops[i]->get)(ptrs[i]);
	mcc_graph_add(MCC_GRAPH(graph[i]), value);
    }
    
    return TRUE;
}

int main(int argc, char **argv)
{
    int i;
    
    gtk_init(&argc, &argv);
    
    for (i = 0; i < NR; i++)
	(*ops[i]->sinit)();
    
    GtkWidget *top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    for (i = 0; i < NR; i++)
	ptrs[i] = (*ops[i]->new)();
    
    for (i = 0; i < NR; i++) {
	graph[i] = mcc_graph_new((*ops[i]->nvalues)(ptrs[i]));
	gtk_container_add(GTK_CONTAINER(top), graph[i]);
    }
    
    gtk_widget_show_all(top);
    
    g_timeout_add(100, timer, NULL);
    
    gtk_main();
    
    for (i = 0; i < NR; i++)
	(*ops[i]->sfini)();
    
    return 0;
}
