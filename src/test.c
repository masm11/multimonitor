#include <gtk/gtk.h>
#include "mccgraph.h"
#include "mccvalue.h"

static gboolean timer(gpointer data)
{
    GtkWidget *g = data;
    
    MccValue *value = mcc_value_new(2);
    mcc_value_set_value(value, 0, rand() % 50);
    mcc_value_set_value(value, 1, rand() % 50);
    mcc_graph_add(MCC_GRAPH(g), value);
    
    g_object_unref(value);
    
    return TRUE;
}

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    
    GtkWidget *top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    GtkWidget *g = mcc_graph_new(2);
    gtk_container_add(GTK_CONTAINER(top), g);
    
    gtk_widget_show_all(top);
    
    g_timeout_add(100, timer, g);
    
    gtk_main();
    
    return 0;
}
