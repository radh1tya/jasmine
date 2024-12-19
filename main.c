#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk-3.0/gtk/gtk.h>

GtkWidget *notebook;

typedef struct Pages{
    GtkWidget *label;
    GtkWidget *textview;
    GtkTextBuffer *buff;
    gboolean saved;
}PgID;
const int SIZE = 10;
PgID *book;

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
    {"Help", 2, {"Contents", "About"}}
};

const int menuLimit = 7;
int order = 0;

void popup_about() {
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "When you smile it\nRain will fall!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void close_window() {
    if (book != NULL) {
        for (int i = 0; i < order; i++) {
            g_object_unref(book[i].buff);
        }
        free(book);
    }
    gtk_main_quit();
}

void save_file(char *file_address) {
    if (file_address) {
        g_print("Saving to: %s\n", file_address);
    }
}

void save_as_dialog() {
    GtkWidget *save_dialog = gtk_file_chooser_dialog_new("Save File", NULL, GTK_FILE_CHOOSER_ACTION_SAVE, 
        "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(save_dialog), TRUE);
    
    if (gtk_dialog_run(GTK_DIALOG(save_dialog)) == GTK_RESPONSE_ACCEPT) {
        char *file_address = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(save_dialog));
        save_file(file_address);
        g_free(file_address);
    }
    gtk_widget_destroy(save_dialog);
}

void open_file(char *file_address) {
    if (file_address) {
        g_print("Opening: %s\n", file_address);
    }
}

void open_file_dialog() {
    GtkWidget *open_dialog = gtk_file_chooser_dialog_new("Open File", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, 
        "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
    
    if (gtk_dialog_run(GTK_DIALOG(open_dialog)) == GTK_RESPONSE_ACCEPT) {
        char *file_address = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(open_dialog));
        open_file(file_address);
        g_free(file_address);
    }
    gtk_widget_destroy(open_dialog);
}

void close_tab(GtkWidget *button, gpointer data) {
    GtkWidget *page = (GtkWidget *)data;
    int page_num = gtk_notebook_page_num(GTK_NOTEBOOK(notebook), page);
    if (page_num != -1) {
        if (book[page_num].saved || !gtk_text_buffer_get_modified(book[page_num].buff)) {
            gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), page_num);
            for (int i = page_num; i < order - 1; i++) {
                book[i] = book[i+1];
            }
            order--;
        } else {
            save_as_dialog();
        }
    }
}

char *name_from_address(const char *address) {
    const char *slash = strrchr(address, '/');
    return slash ? g_strdup(slash + 1) : g_strdup(address);
}

void add_tab(char *address) {
    if (order >= SIZE) {
        g_print("Maximum number of tabs reached!\n");
        return;
    }

    GtkWidget *label = gtk_label_new(NULL);
    GtkWidget *tab_label = gtk_label_new(name_from_address(address));
    GtkWidget *close_button = gtk_button_new_with_label("x");
    gtk_widget_set_tooltip_text(close_button, "Close tab");

    GtkWidget *scrollwin = gtk_scrolled_window_new(NULL, NULL);
    book[order].textview = gtk_text_view_new();
    book[order].buff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(book[order].textview));
    book[order].saved = FALSE;

    GtkWidget *tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_box_pack_start(GTK_BOX(tab_box), tab_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(tab_box), close_button, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(scrollwin), book[order].textview);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrollwin, tab_box);

    g_signal_connect(close_button, "clicked", G_CALLBACK(close_tab), scrollwin);

    gtk_widget_show_all(tab_box);
    gtk_widget_show_all(scrollwin);
    order++;
}

void button_click(GtkWidget *widget, gpointer data) {
    const char *btn = (char*)data;

    if(strcmp(btn, "New") == 0) {
        char *tab_name = g_strdup_printf("new_tab %d", order);
        add_tab(tab_name);
        g_free(tab_name);
    } else if(strcmp(btn, "About") == 0) {
        popup_about();
    } else if(strcmp(btn, "Open") == 0) {
        open_file_dialog();
    } else if(strcmp(btn, "Save") == 0) {
        save_file(NULL); 
    } else if(strcmp(btn, "Save As") == 0) {
        save_as_dialog();
    }
}

void make_notebook(GtkWidget *vbox) {
    notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
    add_tab("untitled");
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
            g_signal_connect(subitem, "activate", G_CALLBACK(button_click), menulist[index].sub_menu[loop]);
            gtk_menu_shell_append(GTK_MENU_SHELL(item_menu), subitem);
        }
    }
    gtk_container_add(GTK_CONTAINER(menubox), menubar);
}

void make_window() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_title(GTK_WINDOW(window), "Jasmine");

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(close_window), NULL);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    make_menu(vbox);
    make_notebook(vbox);
    gtk_widget_show_all(window);
}

int main(int argc, char *argv[]) {
    book = (PgID*)malloc(sizeof(PgID) * SIZE);
    if (book == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    gtk_init(&argc, &argv);
    make_window();
    gtk_main();
    return 0;
}
