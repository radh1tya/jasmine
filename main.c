#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk-3.0/gtk/gtk.h>

GtkWidget *notebook;

typedef struct Pages{
    GtkWidget *text;
    GtkWidget *address;
    GtkTextBuffer *buff;
    gboolean saved;
} PgID;

const int SIZE = 10;
PgID *book;

typedef struct {
    char *label;
    int sub_items;
    char sub_menu[6][15];
} menuButton;

menuButton menulist[] = {
    {"File", 6, {"New", "Open", "Save", "Save As", "Close", "Quit"}},
    {"Edit", 4,{"Cut", "Copy", "Paste", "Delete"}},
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

int close_file_confirmation (int pg_num) {
    GtkDialogFlags flags = GTK_DIALOG_MODAL;
    GtkWidget *confirm_dialog = gtk_message_dialog_new(NULL, flags,
						       GTK_MESSAGE_WARNING,
						       GTK_BUTTONS_NONE,
						       "Closing Unsaved Document");
    gtk_dialog_add_buttons(GTK_DIALOG(confirm_dialog), "Close and Discard",
			   GTK_RESPONSE_CLOSE,
			   "Save and Close",
			   GTK_RESPONSE_ACCEPT,
			   "Cancel",
			   GTK_RESPONSE_CANCEL,
			   NULL);
    int res = gtk_dialog_run(GTK_DIALOG(confirm_dialog));
    int value = 2;
    if(res == GTK_RESPONSE_CLOSE) {
	value = 0;
    }
   if (res == GTK_RESPONSE_ACCEPT) {
	value = 1;
    }

    gtk_widget_destroy(confirm_dialog);
    return value;
}

void close_tab(GtkWidget *button, gpointer data) {
    int pg_num = gtk_notebook_page_num(GTK_NOTEBOOK(notebook), data);
    int limit = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
    int close = 0;
    
    if (gtk_text_buffer_get_modified(book[pg_num].buff))
	close = close_file_confirmation(pg_num);

	switch (close) {
	case 0:
            gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), pg_num);
            for (int i = pg_num; i < limit; i++) {
                book[i] = book[i+1];
	    }
	    order--;
break;
	case 1:
	    save_as_dialog();
	    break;
	default:
	    break;
	}
}
char *name_from_address(char *address) {
    const char *slash = strrchr(address, '/');
    return slash ? g_strdup(slash + 1) : g_strdup(address);
}

void add_tab (char *address) {
    int pg_num = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
    if (pg_num == SIZE) {
	return;
    }
	GtkWidget *text = gtk_label_new(address);
	GtkWidget *label = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *button = gtk_button_new_with_label("x");
	gtk_widget_set_tooltip_text(button, "Close Tab");

	GtkWidget *textview = gtk_text_view_new();

	book[pg_num].text = gtk_label_new(name_from_address(address));
	book[pg_num].address  = gtk_label_new(address);
	book[pg_num].buff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	
        gtk_box_pack_start(GTK_BOX(label), book[pg_num].text, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(label), book[pg_num].address, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(label), button, FALSE, FALSE, 0);
	
	GtkWidget *scrollwindow = gtk_scrolled_window_new(NULL,NULL);
	gtk_container_add(GTK_CONTAINER(scrollwindow), textview);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrollwindow, label);
	
	g_signal_connect(GTK_WIDGET(button), "clicked", 
			 G_CALLBACK(close_tab),scrollwindow);
	
	gtk_widget_show(button);
	gtk_widget_show(book[pg_num].text);
	gtk_widget_show_all(scrollwindow);
	order++;
					      }

void button_click(GtkWidget *widget, gpointer data) {
char *btn = (char*)data;

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
