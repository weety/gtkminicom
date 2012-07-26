#ifndef __SERIAL_H__
#define __SERIAL_H__

//#include <vte/vte.h>
#include <glib.h>

//gint serial_fd=0;
//extern gboolean read_thread_loop;
//gboolean read_thread_loop = TRUE; 
gboolean set_serial_init(gchar *device,gint baud_num,gint veri_num,gint data_num,gint stop_num);
void send_serial(gchar *text, guint length);
void close_serial();

//void  read_thread(VteTerminal *display);
void serial_read_thread_create(void);
void serial_read_thread_exit(void);
void serial_thread_check(void);

int register_callback(void (*callback_func)(gchar *text, glong length), gchar *name);
void unregister_callback(gchar *name);
void unregister_all_callback(void);

#endif
