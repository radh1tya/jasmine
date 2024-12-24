#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk-3.0/gtk/gtk.h>

typedef struct Pages{
    GtkWidget *text;
    GtkWidget *address;
    GtkWidget *view;
    GtkTextBuffer *buff;
    int size;
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

typedef enum {
	TEXT_LARGER,
	TEXT_SMALLER,
	TEXT_NORMAL
}TextSize;

GtkWidget *notebook;
GtkWidget *hbox_1;
GtkWidget *hbox_2;
GtkClipboard *clipboard;
GtkEntryBuffer *search_buff;
GtkTextTag *search_tag;

void add_tab(char *name, char *address);
char* name_from_address(char *address);
void save_as_dialog(SaveAsType type);
gboolean delete_tabs ();
int current_tab ();

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

void search_text() {
g_print("something hapened"); // untuk debug gan
int pg = current_tab();
GtkTextIter start, end;
gtk_text_buffer_get_start_iter(book[pg].buff, &start);
gtk_text_buffer_get_end_iter(book[pg].buff, &start);
gtk_text_buffer_remove_tag(book[pg].buff, search_tag, &start, &end);
end = start;
const char *word = gtk_entry_buffer_get_text(search_buff);

gboolean check;
GtkTextSearchFlags flag = GTK_TEXT_SEARCH_CASE_INSENSITIVE;
while(1) {
	check = gtk_text_iter_forward_search(&end, word, flag, &start, &end, NULL);
	if (!check) {
		break;
	}
	gtk_text_buffer_apply_tag(book[pg].buff,search_tag,&start,&end);
}
}

void delete_text (GtkEntryBuffer *buffer, guint pos, gchar *txt, guint n_txt, gpointer data) {
	search_text();
}

void insert_text(GtkEntryBuffer *buffer, guint pos, gchar *txt, guint n_txt, gpointer data) {
	search_text();
}
void search() {
	GtkTextIter start, end;
	gtk_widget_show_all(hbox_1);
	gtk_widget_hide(hbox_2);

	gboolean check;
	int pg = current_tab();
	check = gtk_text_buffer_get_has_selection(book[pg].buff);

	if(check) {
	gtk_text_buffer_get_selection_bounds(book[pg].buff, &start, &end);
	const char *select = gtk_text_buffer_get_text(book[pg].buff, &start, &end, FALSE);
	gtk_entry_buffer_set_text(search_buff, select, -1);
	}

}

void replace() {
	search();
	gtk_widget_show_all(hbox_2);
}

void hide_search() {
	gtk_widget_hide(hbox_1);
	gtk_widget_hide(hbox_2);
}
char* name_from_address(char *address) {
	char *ch = g_path_get_basename(address);
	return ch;
}

void text_size(TextSize txt_sz) {
	int pg = current_tab();
	int sz = book[pg].size;
	switch (txt_sz) {
		case TEXT_LARGER:
			if (sz < 40) {
				sz+=5;
			}
			break;
		case TEXT_SMALLER:
			if (sz > 5) {
				sz-=5;
			}
			break;
		case TEXT_NORMAL:
			sz = 15;
			break;
		default:
			break;
	}
		book[pg].size=sz;
		char num[3];
		sprintf(num, "%d", book[pg].size);
		char name[8] = "size_";
		strncat(name, num, 3);
		gtk_widget_set_name(book[pg].text, name);
		g_print("%s\n", name);
	}
int current_tab () {
	return gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	
}

void paste_text() {
	int pg = current_tab();
	gtk_text_buffer_paste_clipboard(book[pg].buff, clipboard, NULL, TRUE);

}
void cut_copy_delete (int type) {
	gboolean check;
	int pg = current_tab();
	check = gtk_text_buffer_get_has_selection(book[pg].buff);
	if(!check) {
		return;
	}
	clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

	switch (type) {
	case 1:
		gtk_text_buffer_cut_clipboard(book[pg].buff, clipboard, TRUE);
		break;
	case 2:
		gtk_text_buffer_copy_clipboard(book[pg].buff, clipboard);
		break;
	case 3:
		gtk_text_buffer_delete_selection(book[pg].buff, TRUE, TRUE);
		break;
	default:
		break;
	}
}
void write_failed(const char * address) {
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

void write_file (SaveAsType type) {
	GtkTextIter start, end;
	int pg = current_tab();
	gtk_text_buffer_get_start_iter(book[pg].buff, &start);
	gtk_text_buffer_get_end_iter(book[pg].buff, &end);
	const char * file = gtk_text_buffer_get_text(book[pg].buff, &start, &end, FALSE);

	const char *filename = gtk_label_get_text(GTK_LABEL(book[pg].address));

	if (!g_file_set_contents(filename, file, -1, NULL)) {
		write_failed(filename);
		return;
}
   
	gtk_text_buffer_set_modified(book[pg].buff, FALSE);	
	if(type == SAVE_AND_CLOSE) {
		
		int pg = current_tab();
		int limit = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	
		gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), pg);
		for(int i = pg; i < limit; i++) {
			book[i] = book[i+1];
			order--;
			}
			}
}
void save_file(SaveAsType type) {
	int pg = current_tab();
	const char *addrss = gtk_label_get_text(GTK_LABEL(book[pg].address));

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
        int pg = current_tab();
        
        gtk_label_set_text(GTK_LABEL(book[pg].address), addrss);
        gtk_label_set_text(GTK_LABEL(book[pg].text), name_from_address(addrss));

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
		int pg = current_tab();
		gtk_text_buffer_set_text(book[pg].buff,file_buff,-1);
	gtk_text_buffer_set_modified(book[pg].buff,FALSE);
	} 
	
	else {
		open_failed(address);
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

int close_file_confirmation (int pg) {
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
   int current_pg = current_tab();
   int pg;
   if (data) {
	int pg = gtk_notebook_page_num(GTK_NOTEBOOK(notebook), data);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), pg);
   } else {
	   pg = current_pg;
   }
    int limit = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
    int close = 0;
    
    if (gtk_text_buffer_get_modified(book[pg].buff))
	close = close_file_confirmation(pg);

	switch (close) {
	case 0:
            gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), pg);
            for (int i = pg; i < limit; i++) {
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
    int pg = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
    
    if (pg >= SIZE) {
        return;
    }

    book[pg].size = 15;
    book[pg].buff = gtk_text_buffer_new(NULL); 
    book[pg].text = gtk_text_view_new_with_buffer(book[pg].buff);
    book[pg].address = gtk_label_new(address);

    gtk_widget_set_name(book[pg].text, "size_15");
    gtk_widget_set_hexpand(book[pg].text, TRUE);
    gtk_widget_set_vexpand(book[pg].text, TRUE);

    GtkWidget *label = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *button = gtk_button_new_from_icon_name("window-close", GTK_ICON_SIZE_MENU);
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);

    GtkWidget *name_label = gtk_label_new(name);

    GtkTextTagTable *table;
    table = gtk_text_buffer_get_tag_table(book[pg].buff);
    gtk_text_buffer_create_tag(book[pg].buff, "green", "background", "#2c5d30", NULL); 
    search_tag = gtk_text_tag_table_lookup(table, "green");

    gtk_box_pack_start(GTK_BOX(label), name_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(label), book[pg].address, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(label), button, FALSE, FALSE, 0);

    GtkWidget *scrollwindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_container_add(GTK_CONTAINER(scrollwindow), book[pg].text); 

    gtk_widget_show_all(label);
    gtk_widget_show_all(scrollwindow);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrollwindow, label);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), pg);

    gtk_widget_grab_focus(book[pg].view);
    g_signal_connect(button, "clicked", G_CALLBACK(close_tab), GINT_TO_POINTER(pg));
    
    order++;
}


gboolean delete_event (GtkWidget *window, GdkEvent *event, gpointer data) {
	/*
	int limit = gtk_notebook_get_n_pages(GTK_NOTEBOOK(no
				tebook));
	if(limit > 0) {
		int i;
		for(i=limit; i>0; i--) {
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), i-1);
			close_tab(NULL,NULL);
		}
	}
	limit = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));

	if(limit > 0) {
		return TRUE;
	} else {
		free(book);
		return FALSE;
	}
	*/
	return (delete_tabs());
}

gboolean delete_tabs () {
	int limit = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	if(limit > 0) {
		int i;
		for(i=limit; i>0; i--) {
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), i-1);
			close_tab(NULL,NULL);
		}
	}
	limit = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));

	if(limit > 0) {
		return TRUE;
	} else {
		free(book);
		return FALSE;
	}
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
    } else if(strcmp(btn, "Close") == 0) {
	    close_tab (NULL, NULL);
    } else if(strcmp(btn, "Quit") == 0) {
	    if (! delete_tabs()) {
		    gtk_main_quit();
	    }
    } else if(strcmp(btn, "Copy") == 0) {
	    cut_copy_delete(2);
    } else if(strcmp(btn, "Cut") == 0) {
	    cut_copy_delete(1);
    } else if(strcmp(btn, "Delete") == 0) {
	    cut_copy_delete(3);
    } else if(strcmp(btn, "Paste") == 0) {
	    paste_text();
    } else if(strcmp(btn, "Larger Text") == 0) {
	    text_size(TEXT_LARGER);
    } else if(strcmp(btn, "Smaller Text") == 0) {
	    text_size(TEXT_SMALLER);
    } else if(strcmp(btn, "Normal Size") == 0) {
	    text_size(TEXT_NORMAL);
    } else if(strcmp(btn, "Search") == 0) {
	    search();
    } else if(strcmp(btn, "Replace") == 0) {
	    replace();
    }
}

void make_css() {
	GtkCssProvider *provider;
	provider = gtk_css_provider_new();
	GdkDisplay *display = gdk_display_get_default();
	GdkScreen *screen = gdk_display_get_default_screen(display);
	gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

	gtk_css_provider_load_from_path(GTK_CSS_PROVIDER(provider), "./css_size.css", NULL);
	g_print("Css bisa diload\n");

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

void make_search(GtkWidget *box) {
	GtkWidget *caps = gtk_toggle_button_new_with_label("Aa");
	GtkWidget *word = gtk_toggle_button_new_with_label("\"\"");
	GtkWidget *label = gtk_label_new("Search For: ");
	GtkWidget *field = gtk_entry_new();
	GtkWidget *next = gtk_button_new_with_label("Next");
	GtkWidget *previous = gtk_button_new_with_label("Previous");
	GtkWidget *close = gtk_button_new_with_label("x");

	search_buff = gtk_entry_get_buffer(GTK_ENTRY(field));

	g_signal_connect(GTK_ENTRY_BUFFER(search_buff), "insered-text",
			G_CALLBACK(insert_text), NULL);

	g_signal_connect(GTK_ENTRY_BUFFER(search_buff), "deleted-text",
			G_CALLBACK(delete_text), NULL);

	gtk_box_pack_start(GTK_BOX(box), caps, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), word, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box), close, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box), previous, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box), next, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box), field, TRUE, TRUE, 0);

	g_signal_connect(GTK_WIDGET(close), "clicked", G_CALLBACK(hide_search), NULL);
}

void make_replace(GtkWidget *box) {
	GtkWidget *label = gtk_label_new("Replace With: ");
	GtkWidget *field = gtk_entry_new();
	GtkWidget *replace = gtk_button_new_with_label("Replace");
	gtk_widget_set_size_request(replace, 91, 0);
	
	GtkWidget *replace_all = gtk_button_new_with_label("Replace All");
	GtkWidget *close = gtk_button_new_with_label("x");

	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box), replace, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box), replace_all, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box), field, TRUE, TRUE, 0);


}

void make_window() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_title(GTK_WINDOW(window), "Jasmine");
    
    g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(delete_event), NULL);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    make_menu(vbox);
    make_notebook(vbox);

    GtkWidget *paned;
    paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_CONTAINER(vbox), paned);
    hbox_1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    hbox_2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_paned_pack1(GTK_PANED(paned), hbox_1, TRUE, FALSE);
    gtk_paned_pack2(GTK_PANED(paned), hbox_2, TRUE, FALSE);

    make_css();
    
    gtk_widget_show_all(window);

    make_search(hbox_1);
    make_replace(hbox_2);
    
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
