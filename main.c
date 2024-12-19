#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk-3.0/gtk/gtk.h>
#include </usr/include/gtk-3.0/gdk/gdk.h>

void close_window() { gtk_main_quit(); }

void make_window() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_title(GTK_WINDOW(window), "My Text App");

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(close_window), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    gtk_container_add(GTK_CONTAINER(window), vbox);

    gtk_widget_show_all(window);
}
int main (int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    make_window();
    gtk_main();
    return 0;
}
