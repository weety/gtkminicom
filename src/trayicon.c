#include <gtk/gtk.h>
#include "icon.h"

GtkStatusIcon *trayIcon;
static gboolean win_show_status = FALSE;	//TRUE:show FALSE:hiden
//static gint win_x;	//window x position
//static gint win_y;	//window y position

static void trayView(GtkMenuItem *item, gpointer user_data);
static void trayExit(GtkMenuItem *item, gpointer user_data);
static void trayIconActivated(GObject *trayIcon, gpointer data);
static void trayIconPopup(GtkStatusIcon *status_icon, guint button, guint32 activate_time, gpointer popUpMenu);
static void destroy (GtkWidget*, gpointer);
static gboolean delete_event (GtkWidget*, GdkEvent*, gpointer);
static gboolean window_state_event (GtkWidget *widget, GdkEventWindowState *event, gpointer user_data);

void create_trayicon(GtkWidget *window)
{
	GdkPixbuf *icon_pixbuf;
	GtkWidget *menu, *menuItemView, *menuItemExit;

	icon_pixbuf = gdk_pixbuf_new_from_xpm_data (icon_xpm);
	gtk_window_set_icon (GTK_WINDOW(window), icon_pixbuf);
	trayIcon  = gtk_status_icon_new_from_pixbuf (icon_pixbuf);
	gdk_pixbuf_unref(icon_pixbuf);
	menu = gtk_menu_new();
	menuItemView = gtk_menu_item_new_with_label ("打开主窗口");
	menuItemExit = gtk_menu_item_new_with_label ("退出");
	g_signal_connect (G_OBJECT (menuItemView), "activate", 
				G_CALLBACK (trayView), window);
	g_signal_connect (G_OBJECT (menuItemExit), "activate", 
				G_CALLBACK (trayExit), window);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuItemView);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuItemExit);
	gtk_widget_show_all (menu);
    //set tooltip
	gtk_status_icon_set_tooltip (trayIcon, "Gtkminicom");
    //connect handlers for mouse events
	g_signal_connect(G_OBJECT(trayIcon), "activate", 
				G_CALLBACK (trayIconActivated), window);
	g_signal_connect(G_OBJECT (trayIcon), "popup-menu", 
				G_CALLBACK (trayIconPopup), menu);
	gtk_status_icon_set_visible(trayIcon, TRUE); //set icon initially invisible

	g_signal_connect (G_OBJECT (window), "destroy", 
				G_CALLBACK (destroy), NULL);
	g_signal_connect (G_OBJECT (window), "delete_event", 
				G_CALLBACK (delete_event), trayIcon);
	//g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (gtk_main_quit), trayIcon);
	g_signal_connect (G_OBJECT (window), "window-state-event",
				G_CALLBACK (window_state_event), trayIcon);
	win_show_status = TRUE;
	//gtk_window_get_position (GTK_WINDOW(window), &win_x, &win_y);
}

static void trayView(GtkMenuItem *item, gpointer window) 
{
	//gtk_widget_show(GTK_WIDGET(window));
	//gtk_window_move(GTK_WINDOW(window), win_x, win_y);
	win_show_status = TRUE;
	gtk_window_deiconify(GTK_WINDOW(window)); 
	gtk_widget_show(GTK_WIDGET(window));
}

static void trayExit(GtkMenuItem *item, gpointer window) 
{
	printf("exit");
	// gtk_main_quit();
	extern void mainwindow_quit(GtkWindow *window, gpointer data);
	mainwindow_quit(GTK_WINDOW(window), NULL);
}

static void trayIconActivated(GObject *trayIcon, gpointer window)
{
	if(!win_show_status)
	{
		//gtk_widget_show(GTK_WIDGET(window));
		//gtk_window_move(GTK_WINDOW(window), win_x, win_y);
		//printf("set(%d,%d)\n", win_x, win_y);
		win_show_status = TRUE;
		gtk_window_deiconify(GTK_WINDOW(window)); 
		gtk_widget_show(GTK_WIDGET(window));
	}
	else{
		//gtk_window_get_position (GTK_WINDOW(window), &win_x, &win_y);
		//gtk_widget_hide(GTK_WIDGET(window));
		win_show_status = FALSE;
		gtk_window_iconify(GTK_WINDOW(window));
		gtk_widget_hide(GTK_WIDGET(window));
	//	gtk_window_get_position (GTK_WINDOW(window), &win_x, &win_y);
		//printf("get(%d,%d)\n",win_x,win_y);
	}

}

static void trayIconPopup(GtkStatusIcon *status_icon, guint button, guint32 activate_time, gpointer popUpMenu)
{
	gtk_menu_popup(GTK_MENU(popUpMenu), NULL, NULL, 
			gtk_status_icon_position_menu, 
			status_icon, button, activate_time);
}

static void destroy (GtkWidget *window, gpointer data)
{
	gtk_main_quit ();
}

static gboolean delete_event (GtkWidget *window, GdkEvent *event, gpointer trayIcon)
{
	//gtk_window_get_position (GTK_WINDOW(window), &win_x, &win_y);
	//gtk_widget_hide (GTK_WIDGET(window));
	win_show_status = FALSE;
	gtk_window_iconify(GTK_WINDOW(window));
	// gtk_window_get_position (GTK_WINDOW(window), &win_x, &win_y);
	gtk_status_icon_set_visible(GTK_STATUS_ICON(trayIcon), TRUE);
	gtk_widget_hide (GTK_WIDGET(window));
	return TRUE;
}

static gboolean window_state_event (GtkWidget *window, GdkEventWindowState *event, gpointer trayIcon)
{
	if(event->changed_mask == GDK_WINDOW_STATE_ICONIFIED && (event->new_window_state == GDK_WINDOW_STATE_ICONIFIED || event->new_window_state == (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_MAXIMIZED)))
	{
		//gtk_window_get_position (GTK_WINDOW(window), &win_x, &win_y);
		//gtk_widget_hide (GTK_WIDGET(window));
		win_show_status = FALSE;
		//gtk_window_get_position (GTK_WINDOW(window), &win_x, &win_y);
		gtk_window_iconify(GTK_WINDOW(window));
		gtk_status_icon_set_visible(GTK_STATUS_ICON(trayIcon), TRUE);
		gtk_widget_hide (GTK_WIDGET(window));
	}
	else if(event->changed_mask == GDK_WINDOW_STATE_WITHDRAWN && (event->new_window_state == GDK_WINDOW_STATE_ICONIFIED || event->new_window_state == (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_MAXIMIZED)))
	{
		gtk_status_icon_set_visible(GTK_STATUS_ICON(trayIcon), TRUE);//xiugai
	}
	return TRUE;
}
