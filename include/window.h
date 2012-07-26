#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <gtk/gtk.h>

GtkWidget *mainwindow;
GtkWidget    *display;

typedef struct toolbar_widget
{
	GtkWidget *connect,*disconnect;
}toolbar_widget;
toolbar_widget toolbarwidget;

GtkWidget *create_main_window(void);
void mainwindow_quit(GtkWindow *window, gpointer data);
void set_status_message(gchar *msg);
void clear_display(void);

#endif
