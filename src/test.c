#include <gtk/gtk.h>
#include "mccgraph.h"
#include "mccvalue.h"

static gboolean timer(gpointer data)
{
    GtkWidget *g = data;
    
    gdouble **p = cpuload_get();
    
    MccValue *value = mcc_value_new(7);
    gint i;
    for (i = 0; i < 7; i++) {
	mcc_value_set_value(value, i, p[0][i]);
    }
    mcc_graph_add(MCC_GRAPH(g), value);
    
    g_object_unref(value);
    
    return TRUE;
}

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    
    GtkWidget *top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    GtkWidget *g = mcc_graph_new(7);
    gtk_container_add(GTK_CONTAINER(top), g);
    
    gtk_widget_show_all(top);
    
    g_timeout_add(500, timer, g);
    
    gtk_main();
    
    return 0;
}
