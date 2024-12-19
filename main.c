#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk-3.0/gtk/gtk.h>
#include </usr/include/gtk-3.0/gdk/gdk.h>

typedef struct {
    char *label;
    int sub_items;
    char sub_menu[6][15];
} menuButton;

menuButton menulist[] = {
    {"File", 6, {"New", "Open", "Save", "Save As", "Close", "Quit"}},
    {"Edit", 4, {"Cut", "Copy", "Paste", "Delete"}},
    {"View", 4, {"Full Screen", "Larger Text", "Smaller Text", "Normal Size"}},
    {"Search", 4, {"Find", "Find Next", "Find Previous", "Replace"}},
    {"Tools", 1, {"Check Spelling"}},
    {"Documents", 2, {"Close All", "Save All"}},
    {"Help", 2, {"Contents", "About"}}};

const int menuLimit = 7;

void close_window() { gtk_main_quit(); }

void button_click(GtkWidget *button, gpointer data) {
    g_print("Click");
}
void make_menu(GtkWidget *menubox) {
    GtkWidget *menubar = gtk_menu_bar_new();
    for(int index = 0; index < menuLimit; index++) {
	GtkWidget *item = gtk_menu_item_new_with_label(menulist[index].label);
	GtkWidget *item_menu = gtk_menu_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), item_menu);

	for (int loop = 0; loop < menulist[index].sub_items; loop++) {
	    GtkWidget *subitem = gtk_menu_item_new_with_label(menulist[index].sub_menu[loop]);
	    g_signal_connect(GTK_WIDGET(subitem), "activate", G_CALLBACK(button_click), menulist[index].sub_menu[loop]);
	    gtk_menu_shell_append(GTK_MENU_SHELL(item_menu), subitem);
	}
    }
    gtk_container_add(GTK_CONTAINER(menubox), menubar);
}
void input_field(GtkWidget *vbox) {
    GtkWidget *entry =  gtk_entry_new();
    GtkWidget *ok = gtk_button_new_with_label("Ok!");
    GtkWidget *layout = gtk_grid_new();

    gtk_grid_attach(GTK_GRID(layout), entry, 1,1,2,1);
    gtk_grid_attach(GTK_GRID(layout), ok, 2,1,2,1);

    gtk_container_add(GTK_CONTAINER(vbox), layout);
}
void make_window() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *menubox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_title(GTK_WINDOW(window), "Jasmine");

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(close_window), NULL);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    make_menu(vbox);
    input_field(vbox);
    gtk_widget_show_all(window);
}
int main (int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    make_window();
    gtk_main();
    
    return 0;
}
