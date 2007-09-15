#include <gtk/gtk.h>
#include "mccgraph.h"
#include "mccvalue.h"
#include "mcctick.h"
#include "ops.h"

static void *tmp;
    
static gboolean timer(gpointer data)
{
    GtkWidget *g = data;
    
    (*linux_cpuload_ops.sread)();
    
    MccValue *value = (*linux_cpuload_ops.get)(tmp);
    mcc_graph_add(MCC_GRAPH(g), value);
    
    return TRUE;
}

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    
    (*linux_cpuload_ops.sinit)();
    
    GtkWidget *top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    tmp = (*linux_cpuload_ops.new)();
    
    GtkWidget *g = mcc_graph_new((*linux_cpuload_ops.nvalues)(tmp));
    gtk_container_add(GTK_CONTAINER(top), g);
    
    gtk_widget_show_all(top);
    
    g_timeout_add(100, timer, g);
    
    gtk_main();
    
    (*linux_cpuload_ops.sfini)();
    
    return 0;
}
