#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk-3.0/gtk/gtk.h>

typedef struct Pages{
    GtkWidget *text;
    GtkWidget *address;
    GtkTextBuffer *buff;
} PgID;

typedef struct {
    char *label;
    int sub_items;
    char sub_menu[6][15];
} menuButton;

typedef enum {
	SAVE_AND_CLOSE,
	SAVE_AND_CONTINUE,
}SaveAsType;


GtkWidget *notebook;

void add_tab(char *name, char *address);
char* name_from_address(char *address);
void save_as_dialog(SaveAsType type);

PgID *book;


const int SIZE = 10;
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
int order = 1;
void popup_about() {
    GtkWidget *dialog = gtk_message_dialog_new_with_markup(
        NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "<b>Jasmine</b>\nBeta\nJasmine is a simple text editor\n<a href=\"https://github.com/radh1tya/jasmine\">github.com/radh1tya/jasmine</a>");
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

char* name_from_address(char *address) {
	char *ch = g_path_get_basename(address);
	return ch;
}


void write_file (SaveAsType type) {
	GtkTextIter start, end;
	int pg_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	gtk_text_buffer_get_start_iter(book[pg_num].buff, &start);
	gtk_text_buffer_get_end_iter(book[pg_num].buff, &end);
	const char * file = gtk_text_buffer_get_text(book[pg_num].buff, &start, &end, FALSE);

	const char *filename = gtk_label_get_text(GTK_LABEL(book[pg_num].address));
	if (!g_file_set_contents(filename, file, -1, NULL)) {
    g_print("write failed");
    return;
}
   
	gtk_text_buffer_set_modified(book[pg_num].buff, FALSE);	
	if(type == SAVE_AND_CLOSE) {
		
		int pg_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
		int limit = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	
		gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), pg_num);
		for(int i = pg_num; i < limit; i++) {
			book[i] = book[i+1];
			order--;
			}
			}
}
void save_file(SaveAsType type) {
	int pg_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	const char *addrss = gtk_label_get_text(GTK_LABEL(book[pg_num].address));

	if (g_file_test(addrss, G_FILE_TEST_EXISTS)) {
		write_file(type);
	} else {
		save_as_dialog(type);
	}
}

void save_as_dialog(SaveAsType type) {
    GtkWidget *save_dialog = gtk_file_chooser_dialog_new(
        "Save File", NULL, GTK_FILE_CHOOSER_ACTION_SAVE,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Save", GTK_RESPONSE_ACCEPT, NULL);

    GtkFileChooser *chooser = GTK_FILE_CHOOSER(save_dialog);
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);

    int res = gtk_dialog_run(GTK_DIALOG(save_dialog));

    if (res == GTK_RESPONSE_ACCEPT) {
        char *addrss = gtk_file_chooser_get_filename(chooser);
        int pg_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
        
        gtk_label_set_text(GTK_LABEL(book[pg_num].address), addrss);
        gtk_label_set_text(GTK_LABEL(book[pg_num].text), name_from_address(addrss));

        write_file(type);

        g_free(addrss);
    }

    gtk_widget_destroy(save_dialog);
}


void open_failed(char *address) {
	GtkDialogFlags flags = GTK_DIALOG_MODAL;
	GtkWidget *warning_dialog;
	int res;
	warning_dialog = gtk_message_dialog_new(NULL,flags,
			GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
			"Could not Open File\n%s",address
			);
	res = gtk_dialog_run(GTK_DIALOG(warning_dialog));
	gtk_widget_destroy(warning_dialog);
}

void open_file(char *address) {
	//GFile *file;
	//file = g_file_new_for_path(address);
	char *file_buff;
	gboolean check;
	check = g_file_get_contents(address, &file_buff, NULL, NULL);
	if (check && g_utf8_validate(file_buff, -1, NULL)) {
		add_tab(name_from_address(address), address);
		int pg_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
		gtk_text_buffer_set_text(book[pg_num].buff,file_buff,-1);
	gtk_text_buffer_set_modified(book[pg_num].buff,FALSE);
	} 
	
	else {
		g_print("open failed");
	}
	
	g_free(file_buff);
	//g_object_unref(file);
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
	return 0;
    }
   if (res == GTK_RESPONSE_ACCEPT) {
	   return 1;
   }

    gtk_widget_destroy(confirm_dialog);
    return value;
}

void close_tab(GtkWidget *button, gpointer data) {
   int current_pg = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
   int pg_num;
   if (data) {
	int pg_num = gtk_notebook_page_num(GTK_NOTEBOOK(notebook), data);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), pg_num);
   } else {
	   pg_num = current_pg;
   }
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
	    save_file(SAVE_AND_CLOSE);
	    break;
	default:
	    break;
	}
	if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)) > 0) {
	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), current_pg);
	}
}

void add_tab(char *name, char *address) {
    int pg_num = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
    
    if (pg_num >= SIZE) {
        return;
    }

    book[pg_num].buff = gtk_text_buffer_new(NULL); 
    book[pg_num].text = gtk_text_view_new_with_buffer(book[pg_num].buff);
    book[pg_num].address = gtk_label_new(address);
    gtk_widget_set_hexpand(book[pg_num].text, TRUE);
    gtk_widget_set_vexpand(book[pg_num].text, TRUE);

    GtkWidget *label = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *button = gtk_button_new_from_icon_name("window-close", GTK_ICON_SIZE_MENU);
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);

    GtkWidget *name_label = gtk_label_new(name);
    gtk_box_pack_start(GTK_BOX(label), name_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(label), book[pg_num].address, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(label), button, FALSE, FALSE, 0);

    GtkWidget *scrollwindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrollwindow), book[pg_num].text);

    gtk_widget_show_all(label);
    gtk_widget_show_all(scrollwindow);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrollwindow, label);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), pg_num);

    g_signal_connect(button, "clicked", G_CALLBACK(close_tab), GINT_TO_POINTER(pg_num));
    
    order++;
}

void button_click(GtkWidget *widget, gpointer data) {
char *btn = (char*)data;

    if(strcmp(btn, "New") == 0) {
        char *tab_name = g_strdup_printf("untitled %d ", order);
        add_tab(tab_name, "");
        g_free(tab_name);
    } else if(strcmp(btn, "About") == 0) {
        popup_about();
    } else if(strcmp(btn, "Open") == 0) {
        open_file_dialog();
    } else if(strcmp(btn, "Save") == 0) {
        save_file(SAVE_AND_CONTINUE); 
    } else if(strcmp(btn, "Save As") == 0) {
        save_as_dialog(SAVE_AND_CONTINUE);
    }
}

void make_notebook(GtkWidget *vbox) {
    notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
    add_tab("Unsaved Document", "");
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
