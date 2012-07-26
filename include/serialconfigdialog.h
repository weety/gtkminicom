#ifndef __SERIALCONFIGDIALOG_H__
#define __SERIALCONFIGDIALOG_H__

#include <gtk/gtk.h>

//GThread *serial_read_thread = NULL;
void create_serialconfig_dialog(void);
//void serial_read_thread_exit(void);
gboolean serial_init(void);

#endif
