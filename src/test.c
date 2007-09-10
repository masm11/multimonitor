#include <gtk/gtk.h>
#include "mccgraph.h"

static gboolean timer(gpointer data)
{
    GtkWidget *g = data;
    
    mcc_graph_add(g, rand() % 200);
    
    return TRUE;
}

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    
    GtkWidget *top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    GtkWidget *g = mcc_graph_new();
    gtk_container_add(GTK_CONTAINER(top), g);
    
    gtk_widget_show_all(top);
    
    g_timeout_add(1000, timer, g);
    
    gtk_main();
    
    return 0;
}
