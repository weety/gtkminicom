#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <gtk/gtk.h>
#include <vte/vte.h>
#include "window.h"
#include "serialconfigdialog.h"
#include "serial.h"
#include "logfile.h"

//GtkBuilder   *builder;
//GtkWidget    *display;
guint id;
GtkWidget    *statusbar;

struct sigaction sigwaitchild;
void signal_handler_wait_child (int status);   /* definition of signal handler */  
                                     // 定义信号处理程序，子进程退出时的信号处理函数

void display_write_data(gchar *text, glong length);


void on_clear_active(GtkMenuItem *item, gpointer data);
static void on_input(VteTerminal *widget, gchar *text, guint length, gpointer ptr);
void on_serialconfig_active(GtkMenuItem *item, gpointer data);

static void main_quit(void)
{
	serial_read_thread_exit();
	unregister_all_callback();
	//close_serial();
	logfile_close();
	if(unlink(filepath)==-1)
		printf("delete logfile failed\n");
	gtk_main_quit();

}

void on_menu_active(GtkMenuItem *item, gpointer data)
{
	g_print("Menu item %s is pressed.\n", (gchar *)data);
	//set_status_message((gchar *)data);
}


void on_serialconfig_active(GtkMenuItem *item, gpointer data)
{
	create_serialconfig_dialog();
}


void on_disconnect_active(GtkMenuItem *item, gpointer data)
{
	serial_read_thread_exit();
	//g_printf("Menu item %s is pressed.\n", (gchar *)data);
	set_status_message((gchar *)data);
}


void on_connect_active(GtkMenuItem *item, gpointer data)
{
	serial_init();
	//g_printf("Menu item %s is pressed.\n", (gchar *)data);
	//set_status_message((gchar *)data);
}

void on_about_active(GtkMenuItem *item, gpointer data)
{
	GtkWidget  *about;
	const gchar *authors = g_locale_to_utf8("hui luo <luohuia@yahoo.com.cn>\n", -1, NULL, NULL, NULL);
	//g_printf("Menu item %s is pressed.\n", (gchar *)data);
	//set_status_message((gchar *)data);
	about = gtk_about_dialog_new ();
	gtk_about_dialog_set_name (GTK_ABOUT_DIALOG(about), "About gtkminicom");
	gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG(about), "gtkminicom");
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(about), "v0.6");
	gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG(about), "Copyright © 2008–2010 luohui");
	gtk_about_dialog_set_license (GTK_ABOUT_DIALOG(about), "gtkminicom是自由软件；您可以在自由软件基金会发布\n的 GNU 通用公共许可证下重新发布或修改它；许可证应使\n用第二版本或您所选择的更新的版本。\n发布 gtkminicom的目的是希望它能够在一定程度上帮到\n您。但我并不为它提供任何形式的担保，也无法保证它可\n以在特定用途中得到您希望的结果。请参看 GNU GPL 许\n可中的更多细节。\n您应该在接受gtkminicom的同时接受 GNU GPL协议\n的副本；如果您不接受的话，请给自由软件基金会写信，\n地址是 51 Franklin Street, Fifth Floor, Boston, \nMA  02110-1301  USA");
	//gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG(about), &authors);//可能导致崩溃
	gtk_window_set_transient_for (GTK_WINDOW(about), GTK_WINDOW(mainwindow));
	gtk_window_set_destroy_with_parent (GTK_WINDOW(about), TRUE);
	gtk_dialog_run (GTK_DIALOG(about));
	gtk_widget_destroy(about);
}

void do_menuquit_active(GtkMenuItem *item, gpointer data)
{
	main_quit();
}

#if 0
static GtkItemFactoryEntry menu_items[]=
{
	{"/文件(_F)", NULL, NULL, 0, "<Branch>"},
	{"/文件(_F)/清除显示", NULL, on_clear_active, 0, "<StockItem>", GTK_STOCK_CLEAR},
	{"/文件(_F)/连接", NULL, on_connect_active, 0, "<StockItem>", GTK_STOCK_CONNECT},
	{"/文件(_F)/断开", NULL, on_disconnect_active, 0, "<StockItem>", GTK_STOCK_DISCONNECT},
	{"/文件(_F)/退出", NULL, do_menuquit_active, 0, "<StockItem>", GTK_STOCK_QUIT},
	{"/配置(_S)", NULL, NULL, 0, "<Branch>"},
	{"/配置(_S)/串口配置", NULL, on_serialconfig_active, 0, "<StockItem>", GTK_STOCK_PREFERENCES},
	//{"/配置(_S)/终端设置", NULL, on_menu_active, "终端设置", "<StockItem>", GTK_STOCK_PROPERTIES},
	{"/帮助(_H)", NULL, NULL, 0, "<Branch>"},
	{"/帮助(_H)/关于", NULL, on_about_active, 0, "<StockItem>", GTK_STOCK_ABOUT}
};
#define MENU_ITEMS_NUM 9
#endif

static const gchar *ui_info = 
"<ui>"
"  <menubar name='MenuBar'>"
"    <menu action='file'>"
"      <menuitem action='clear_display'/>"
"      <menuitem action='connect'/>"
"      <menuitem action='disconnect'/>"
"      <separator/>"
"      <menuitem action='quit'/>"
"    </menu>"
"    <menu action='config'>"
"	<menuitem action='serial_config'/>"
"    </menu>"
"    <menu action='help'>"
"      <menuitem action='about'/>"
"    </menu>"
"  </menubar>"
//"  <toolbar  name='ToolBar'>"
//"    <toolitem action='Open'/>"
//"    <toolitem action='Quit'/>"
//"    <separator action='Sep1'/>"
//"    <toolitem action='Logo'/>"
//"  </toolbar>"
"</ui>";

static GtkActionEntry entries[] = {
	{ "file", NULL, "文件(_F)" },
	{ "clear_display", GTK_STOCK_CLEAR, "清除显示",
	NULL, "清除显示", G_CALLBACK(on_clear_active)},
	{ "connect", GTK_STOCK_CONNECT, "连接",
	NULL, "连接", G_CALLBACK(on_connect_active)},
	{ "disconnect", GTK_STOCK_DISCONNECT, "断开",
	NULL, "断开", G_CALLBACK(on_disconnect_active)},
	{ "quit", GTK_STOCK_QUIT, "退出",
	"<control>Q", "退出", G_CALLBACK(do_menuquit_active)},
	{ "config", NULL, "配置(_S)" },
	{ "serial_config", GTK_STOCK_CLEAR, "串口配置",
	"<control>S", "串口配置", G_CALLBACK(on_serialconfig_active)},
	{ "help", NULL, "帮助(_H)" },
	{ "about", GTK_STOCK_CLEAR, "关于",
	NULL, "关于", G_CALLBACK(on_about_active)}
};
#define MENU_GROUP_NUM 9


void set_status_message(gchar *msg)
{
	gtk_statusbar_pop(GTK_STATUSBAR(statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), id, msg);
}

void clear_display(void)
{
	//  initialize_hexadecimal_display();
	if(display)
		vte_terminal_reset(VTE_TERMINAL(display), TRUE, TRUE);
}

void on_clear_active(GtkMenuItem *item, gpointer data)
{
	//g_printf("Menu item %s is pressed.\n", (gchar *)data);
	set_status_message((gchar *)data);
	clear_display();
}



void on_serialconfig_clicked(GtkButton *button, gpointer data)
{
	create_serialconfig_dialog();
}


void on_serialconnect_clicked(GtkButton *button, gpointer data)
{
	if(serial_init()==TRUE)
	{
		gtk_widget_set_sensitive (GTK_WIDGET(data), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(button), FALSE);
	}
}

void on_serialdisconnect_clicked(GtkButton *button, gpointer data)
{
	serial_read_thread_exit();
	set_status_message("断开");
	gtk_widget_set_sensitive (GTK_WIDGET(button), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(data), TRUE);
}

void create_toolbar(GtkWidget *toolbar)
{
	gtk_toolbar_insert_stock (GTK_TOOLBAR(toolbar),
                                   GTK_STOCK_PREFERENCES,
                                   "串口设置",
                                   "设置",
                                   G_CALLBACK(on_serialconfig_clicked),
                                   (gpointer)("设置"),
                                   -1);
	toolbarwidget.connect = gtk_toolbar_insert_stock (GTK_TOOLBAR(toolbar),
						GTK_STOCK_CONNECT,
						"连接",
						"连接",
						G_CALLBACK(on_serialconnect_clicked),
						(gpointer)(toolbarwidget.disconnect),
						-1);
	toolbarwidget.disconnect = gtk_toolbar_insert_stock (GTK_TOOLBAR(toolbar),
						GTK_STOCK_DISCONNECT,
						"断开",
						"断开",
						G_CALLBACK(on_serialdisconnect_clicked),
						(gpointer)(toolbarwidget.connect),
						-1);
	gtk_toolbar_set_style (GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_icon_size (GTK_TOOLBAR(toolbar), GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_widget_set_sensitive (toolbarwidget.disconnect, FALSE);
}


static gint menu_popup_handler (GtkWidget *widget, GdkEvent *event)
{
	GtkMenu *menu;
	GdkEventButton *event_button;
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_MENU (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	/* The "widget" is the menu that was supplied when
	* g_signal_connect_swapped() was called.
	*/
	menu = GTK_MENU (widget);
	if (event->type == GDK_BUTTON_PRESS)
	{
		event_button = (GdkEventButton *) event;
		if (event_button->button == 3)
		{
			gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
			  event_button->button, event_button->time);
			return TRUE;
		}
	}
	return FALSE;
}


void popup_menu_selectall_clicked(GtkImageMenuItem *menuitem, gpointer data)
{
	//g_print("selectall active\n");
	vte_terminal_select_all (VTE_TERMINAL(display));
}

void popup_menu_copy_clicked(GtkImageMenuItem *menuitem, gpointer data)
{
	//g_print("copy active\n");
	vte_terminal_copy_clipboard (VTE_TERMINAL(display));
}

void popup_menu_paste_clicked(GtkImageMenuItem *menuitem, gpointer data)
{
	//g_print("paste active\n");
	vte_terminal_paste_clipboard (VTE_TERMINAL(display));
}

void signal_handler_wait_child (int status)
{
	wait(NULL);
}

void popup_menu_edit_logfile_clicked(GtkImageMenuItem *menuitem, gpointer data)
{
	//g_print("paste active\n");
	pid_t pid;
	struct stat dev_stat;
	if(stat(filepath, &dev_stat) == 0)
	{
		logfile_refresh();
		if((pid = fork()) == 0)
		{
			//execl("/usr/bin/gedit", "/usr/bin/gedit", filepath, NULL);
			execl("/usr/bin/xdg-open", "/usr/bin/xdg-open", filepath, NULL);
		}
		sigwaitchild.sa_handler = signal_handler_wait_child;
		sigwaitchild.sa_flags = 0;     
		sigwaitchild.sa_restorer = NULL;    
		sigaction(SIGCHLD,&sigwaitchild,NULL);
	}
}


void do_clear_logfile(GtkImageMenuItem *menuitem, gpointer window)
{
	//FILE *fd;
	gchar  *message;
	gint   ret;
	GtkWidget  *clear_logfile_dialog;
	message = g_strdup_printf("确实要清空日志文件?");
	clear_logfile_dialog = gtk_message_dialog_new (GTK_WINDOW(window),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_OK_CANCEL,
					message);
	ret = gtk_dialog_run (GTK_DIALOG(clear_logfile_dialog));
	gtk_widget_destroy(clear_logfile_dialog);
	g_free(message);
	if(ret == GTK_RESPONSE_OK)
	{
		//fd = fopen(filepath, "w");
		if (clear_logfile())
		{
			message = g_strdup_printf("清空日志文件失败！");
			clear_logfile_dialog = gtk_message_dialog_new (GTK_WINDOW(window),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					message);
			gtk_dialog_run (GTK_DIALOG(clear_logfile_dialog));
			gtk_widget_destroy(clear_logfile_dialog);
			g_free(message);
		}
		else
		{
			//fclose(fd);
			message = g_strdup_printf("清空日志文件成功！");
			clear_logfile_dialog = gtk_message_dialog_new (GTK_WINDOW(window),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_INFO,
					GTK_BUTTONS_OK,
					message);
			gtk_dialog_run (GTK_DIALOG(clear_logfile_dialog));
			gtk_widget_destroy(clear_logfile_dialog);
			g_free(message);
		}
	}
}

void do_save_logfile(GtkImageMenuItem *menuitem, gpointer window)
{
	FILE *in_fd,*out_fd;
	char *filename;
	char data_buf[128];
	int  data_len;
	GtkWidget *save_logfile_dialog;
	save_logfile_dialog = gtk_file_chooser_dialog_new("保存日志文件",
                                                         GTK_WINDOW(window),
                                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							 GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                         NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (save_logfile_dialog), TRUE);
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (save_logfile_dialog), (const char *)getenv("HOME"));
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (save_logfile_dialog), "Untitled");
	if (gtk_dialog_run (GTK_DIALOG (save_logfile_dialog)) == GTK_RESPONSE_ACCEPT)
	{
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (save_logfile_dialog));
		logfile_refresh();
		in_fd = fopen(filepath, "r");
		out_fd = fopen(filename, "w");
		do
		{
			data_len = fread(data_buf, sizeof(char), 128, in_fd);
			fwrite(data_buf, sizeof(char), data_len, out_fd);
		}while(!feof(in_fd));
		fclose(in_fd);
		fclose(out_fd);
		g_free (filename);
	}
	gtk_widget_destroy (save_logfile_dialog);
}


GtkWidget *create_popup_menu(GtkWidget *window)
{
	GtkWidget *menu;
	GtkWidget *menuitem;
	GtkAccelGroup *accel_group;
	accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
	menu = gtk_menu_new();

	menuitem =  gtk_image_menu_item_new_with_label("保存日志文件");
	gtk_menu_shell_prepend (GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(do_save_logfile), window);
	gtk_widget_show(menuitem);

	menuitem =  gtk_image_menu_item_new_with_label("清空日志文件");
	gtk_menu_shell_prepend (GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(do_clear_logfile), window);
	gtk_widget_show(menuitem);
#if 0
	menuitem =  gtk_image_menu_item_new_from_stock(GTK_STOCK_EDIT, NULL);
	gtk_menu_shell_prepend (GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(popup_menu_edit_logfile_clicked), NULL);
	gtk_widget_show(menuitem);
#endif
	menuitem =  gtk_image_menu_item_new_with_label("编辑日志文件");
	gtk_menu_shell_prepend (GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(popup_menu_edit_logfile_clicked), NULL);
	gtk_widget_show(menuitem);

	menuitem =  gtk_image_menu_item_new_from_stock(GTK_STOCK_SELECT_ALL, accel_group);
	gtk_menu_shell_prepend (GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(popup_menu_selectall_clicked), NULL);
	gtk_widget_show(menuitem);
	
	menuitem =  gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, NULL);
	gtk_menu_shell_prepend (GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(popup_menu_copy_clicked), NULL);
	gtk_widget_show(menuitem);
	
	menuitem =  gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, accel_group);
	gtk_menu_shell_prepend (GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(popup_menu_paste_clicked), NULL);
	gtk_widget_show(menuitem);

	return menu;
}


void mainwindow_quit(GtkWindow *window, gpointer data)
{
//	close_serial();
//	fclose(logfilefd);
//	if(unlink(filepath)==-1)
//		printf("delete logfile failed\n");
//	gtk_main_quit();
	main_quit();
}



GtkWidget *create_main_window(void)
{
	GtkWidget *window;
	GtkWidget *vbox1;
	GtkWidget *scrolledwindow;
	GtkWidget *menubar;
//	GtkAccelGroup *accel_group;
//	GtkItemFactory *item_factory;
	//GtkWidget *menubox;
	GtkWidget *toolbar;
	GtkWidget *menu;
	
	GtkActionGroup *actiongroup;
	GtkUIManager *ui;
	
	GError *error = NULL;
   
	//builder = gtk_builder_new ();
	//gtk_builder_add_from_file (builder, "gtkminicom.glade", NULL);
	//window = GTK_WIDGET (gtk_builder_get_object (builder, "mainwindow"));
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW(window), "gtkminicom");
	gtk_window_set_resizable (GTK_WINDOW(window), TRUE);
	gtk_window_set_default_size (GTK_WINDOW(window), 700, 550);
	gtk_window_set_position (GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	mainwindow = window;
//	g_signal_connect(G_OBJECT(window),"delete_event",G_CALLBACK(mainwindow_quit),NULL);

	vbox1 = gtk_vbox_new (FALSE, 1);
	gtk_container_add(GTK_CONTAINER(window), vbox1);
	
	actiongroup = gtk_action_group_new("Actions");
	gtk_action_group_add_actions(actiongroup, entries, MENU_GROUP_NUM, NULL);
 
	ui = gtk_ui_manager_new();
	gtk_ui_manager_insert_action_group(ui, actiongroup, 0);
	g_object_unref (actiongroup);
	
	gtk_window_add_accel_group (GTK_WINDOW (window), 
				  gtk_ui_manager_get_accel_group (ui));
	
	if (!gtk_ui_manager_add_ui_from_string (ui, ui_info, -1, &error))
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	menubar = gtk_ui_manager_get_widget (ui, "/MenuBar");
#if 0
	accel_group = gtk_accel_group_new();
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);
	gtk_item_factory_create_items(item_factory, MENU_ITEMS_NUM, menu_items, NULL);
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
	menubar = gtk_item_factory_get_widget(item_factory, "<main>");
	//menubox = GTK_WIDGET (gtk_builder_get_object (builder, "menubox"));
	//gtk_box_pack_start(GTK_BOX(menubox), menubar, FALSE, FALSE, 0);
#endif
	gtk_box_pack_start(GTK_BOX(vbox1), menubar, FALSE, FALSE, 0);

	//toolbar = GTK_WIDGET (gtk_builder_get_object (builder, "toolbar"));
	toolbar = gtk_toolbar_new();
	create_toolbar(toolbar);
	gtk_box_pack_start(GTK_BOX(vbox1), toolbar, FALSE, FALSE, 0);
     
     
	//scrolledwindow = GTK_WIDGET (gtk_builder_get_object (builder, "scrolledwindow"));
	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_box_pack_start(GTK_BOX(vbox1), scrolledwindow, TRUE, TRUE, 0);
	display = vte_terminal_new();      
	vte_terminal_set_backspace_binding(VTE_TERMINAL(display),
				     VTE_ERASE_ASCII_BACKSPACE);
	vte_terminal_set_default_colors (VTE_TERMINAL(display));
	gtk_container_add(GTK_CONTAINER(scrolledwindow), display);
	//gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolledwindow), display);
	vte_terminal_set_scrollback_lines   (VTE_TERMINAL(display), 8192);
    
	//    vte_terminal_feed(VTE_TERMINAL(display), "hello!\r\n", 9);
	
	//vte_terminal_fork_command(VTE_TERMINAL(display), NULL, NULL, 
        //                          NULL, NULL, TRUE, TRUE, TRUE);
    

    
	//statusbar = GTK_WIDGET (gtk_builder_get_object (builder, "statusbar1"));
	statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox1), statusbar, FALSE, FALSE, 0);
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "messages");
    
	g_signal_connect_after(G_OBJECT(display), "commit", G_CALLBACK(on_input), NULL);
	
	menu = create_popup_menu(window);
	g_signal_connect_swapped(G_OBJECT(display), "button_press_event", G_CALLBACK(menu_popup_handler), G_OBJECT(menu));
    
	gtk_widget_show (menubar);
	gtk_widget_show (toolbar);
	gtk_widget_show (display);
	gtk_widget_show (scrolledwindow);
	gtk_widget_set_can_default (display, TRUE);
	gtk_widget_grab_default (display);
	gtk_widget_grab_focus (display);
	//gtk_builder_connect_signals (builder, NULL); 
	//g_object_unref (G_OBJECT (builder));
	
	g_object_unref (ui);
  
	register_callback(display_write_data, "display");
	
	return window;

}

static void on_input(VteTerminal *widget, gchar *text, guint length, gpointer ptr)
{
	send_serial(text, length);
	//printf("vte input");
	//  send_serial(text, length);
	//vte_terminal_feed(VTE_TERMINAL(display), text, length);
	//vte_terminal_feed(VTE_TERMINAL(display), "\r\n", 2);
	
}


void display_write_data(gchar *text, glong length)
{
    vte_terminal_feed(VTE_TERMINAL(display), text, length);
}
