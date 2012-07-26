#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include "serialconfigdialog.h"
#include "serial.h"
#include "window.h"
#include "trayicon.h"

//#define G_THREAD_FUNC(f) ((GThreadFunc) (f))
#define MAX_SERIAL_DEVICE 16

//extern gboolean read_thread_loop;

static gchar *device_name;
static gint baud_num,veri_num,data_num,stop_num;

//GtkBuilder *serialdailog_builder;
GtkWidget  *table;
GtkWidget  *serial_number;
GtkWidget  *bitrate;
GtkWidget  *checkbit;
GtkWidget  *databit;
GtkWidget  *stopbit;
GtkWidget  *flowcontrol;

void ok_button_clicked(GtkButton *button, gpointer data);
void cancel_button_clicked(GtkButton *button, gpointer data);


void create_serialconfig_dialog(void)
{
//	GtkBuilder *serialdailog_builder;
	GtkWidget  *serialconfig_dialog;
	GtkWidget  *dialog_vbox1;
	GtkWidget  *frame;
	GtkWidget  *dialog_action_area1;
	GtkWidget  *ok_button;
	GtkWidget  *cancel_button;
//	GtkWidget  *serial_number;
	GtkWidget  *label;
	struct stat dev_stat;
	gchar device[64];
	int i;
	
	//serialdailog_builder = gtk_builder_new ();
	//gtk_builder_add_from_file (serialdailog_builder, "serialconfig.glade", NULL);
	//serialconfig_dialog = GTK_WIDGET (gtk_builder_get_object (serialdailog_builder, "serialconfig_dialog"));
	serialconfig_dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_transient_for (GTK_WINDOW(serialconfig_dialog), GTK_WINDOW(mainwindow));
	gtk_window_set_destroy_with_parent (GTK_WINDOW(serialconfig_dialog), TRUE);
	gtk_window_set_modal(GTK_WINDOW(serialconfig_dialog), TRUE);
	gtk_window_set_position (GTK_WINDOW(serialconfig_dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_title (GTK_WINDOW(serialconfig_dialog), "serial config");
	gtk_window_set_resizable (GTK_WINDOW(serialconfig_dialog), FALSE);
	g_signal_connect(G_OBJECT(serialconfig_dialog),"delete_event",G_CALLBACK(gtk_widget_destroy),NULL);
	
	dialog_vbox1 = gtk_vbox_new (FALSE, 1);
	gtk_box_set_spacing (GTK_BOX(dialog_vbox1), 2);
	gtk_container_add(GTK_CONTAINER(serialconfig_dialog), dialog_vbox1);
	gtk_container_set_border_width (GTK_CONTAINER(serialconfig_dialog), 3);
	
	frame = gtk_frame_new("串口设置");
	gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
	gtk_box_pack_start(GTK_BOX(dialog_vbox1), frame, TRUE, TRUE, 0);
	
	//table = GTK_WIDGET (gtk_builder_get_object (serialdailog_builder, "table"));
	table = gtk_table_new(6, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE(table), 1);
	gtk_table_set_col_spacings (GTK_TABLE(table), 3);
	gtk_container_add(GTK_CONTAINER(frame), table);
	gtk_container_set_border_width (GTK_CONTAINER(frame), 2);
	
	dialog_action_area1 = gtk_hbutton_box_new ();
	gtk_hbutton_box_set_layout_default (GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX(dialog_action_area1), 2);
	gtk_box_pack_start(GTK_BOX(dialog_vbox1), dialog_action_area1, TRUE, FALSE, 0);

	//ok_button = GTK_WIDGET (gtk_builder_get_object (serialdailog_builder, "ok_button"));
	ok_button = gtk_button_new_with_label("确定");
	g_signal_connect(G_OBJECT(ok_button),"clicked",G_CALLBACK(ok_button_clicked),G_OBJECT(serialconfig_dialog));
	//cancel_button = GTK_WIDGET (gtk_builder_get_object (serialdailog_builder, "cancel_button"));
	cancel_button = gtk_button_new_with_label("取消");
	g_signal_connect(G_OBJECT(cancel_button),"clicked",G_CALLBACK(cancel_button_clicked),G_OBJECT(serialconfig_dialog));

	gtk_box_pack_start(GTK_BOX(dialog_action_area1), ok_button, FALSE, TRUE, 0);	
	gtk_box_pack_start(GTK_BOX(dialog_action_area1), cancel_button, FALSE, TRUE, 0);

	
//	serial_number = GTK_WIDGET (gtk_builder_get_object (serialdailog_builder, "serial_number"));
	label = gtk_label_new("串口号：");
	gtk_table_attach ( GTK_TABLE(table),
                           label,
                           0,1,0,1,
                           GTK_SHRINK, GTK_SHRINK,
                           0,0);
	serial_number =  gtk_combo_box_new_text();
	gtk_table_attach ( GTK_TABLE(table),
                           serial_number,
                           1,2,0,1,
                           GTK_SHRINK, GTK_SHRINK,
                           0,0);
	for(i=0; i<MAX_SERIAL_DEVICE; i++)
	{
		sprintf(device,"/dev/ttyS%d",i);
		if(stat(device, &dev_stat) == 0)
		{
			gtk_combo_box_append_text (GTK_COMBO_BOX(serial_number),device);
		}
	}
	for(i=0; i<MAX_SERIAL_DEVICE; i++)
	{
		sprintf(device,"/dev/ttyUSB%d",i);
		if(stat(device, &dev_stat) == 0)
		{
			gtk_combo_box_append_text (GTK_COMBO_BOX(serial_number),device);
		}
	}
	//gtk_combo_box_append_text (GTK_COMBO_BOX(serial_number),"/dev/ttyS0");
	//gtk_combo_box_append_text (GTK_COMBO_BOX(serial_number),"/dev/ttyS1");
	//gtk_combo_box_append_text (GTK_COMBO_BOX(serial_number),"/dev/ttyS2");
	//gtk_combo_box_append_text (GTK_COMBO_BOX(serial_number),"/dev/ttyS3");
	gtk_combo_box_set_active (GTK_COMBO_BOX(serial_number), 0);
	
	label = gtk_label_new("波特率：");
	gtk_table_attach ( GTK_TABLE(table),
                           label,
                           0,1,1,2,
                           GTK_SHRINK, GTK_SHRINK,
                           0,0);
	bitrate =  gtk_combo_box_new_text();
	gtk_table_attach ( GTK_TABLE(table),
                           bitrate,
                           1,2,1,2,
                           GTK_SHRINK, GTK_SHRINK,
                           0,0);
	gtk_combo_box_append_text (GTK_COMBO_BOX(bitrate),"300");
	gtk_combo_box_append_text (GTK_COMBO_BOX(bitrate),"600");
	gtk_combo_box_append_text (GTK_COMBO_BOX(bitrate),"1200");
	gtk_combo_box_append_text (GTK_COMBO_BOX(bitrate),"2400");
	gtk_combo_box_append_text (GTK_COMBO_BOX(bitrate),"4800");
	gtk_combo_box_append_text (GTK_COMBO_BOX(bitrate),"9600");
	gtk_combo_box_append_text (GTK_COMBO_BOX(bitrate),"19200");
	gtk_combo_box_append_text (GTK_COMBO_BOX(bitrate),"38400");
	gtk_combo_box_append_text (GTK_COMBO_BOX(bitrate),"57600");
	gtk_combo_box_append_text (GTK_COMBO_BOX(bitrate),"115200");
	gtk_combo_box_set_active (GTK_COMBO_BOX(bitrate), 9);
	
	label = gtk_label_new("校验位：");
	gtk_table_attach ( GTK_TABLE(table),
                           label,
                           0,1,2,3,
                           GTK_SHRINK, GTK_SHRINK,
                           0,0);
	checkbit =  gtk_combo_box_new_text();
	gtk_table_attach ( GTK_TABLE(table),
                           checkbit,
                           1,2,2,3,
                           GTK_SHRINK, GTK_SHRINK,
                           0,0);
	gtk_combo_box_append_text (GTK_COMBO_BOX(checkbit),"none");
	gtk_combo_box_append_text (GTK_COMBO_BOX(checkbit),"odd");
	gtk_combo_box_append_text (GTK_COMBO_BOX(checkbit),"even");
	gtk_combo_box_append_text (GTK_COMBO_BOX(checkbit),"mark");
	gtk_combo_box_append_text (GTK_COMBO_BOX(checkbit),"space");
	gtk_combo_box_set_active (GTK_COMBO_BOX(checkbit), 0);
	
	label = gtk_label_new("数据位：");
	gtk_table_attach ( GTK_TABLE(table),
                           label,
                           0,1,3,4,
                           GTK_SHRINK, GTK_SHRINK,
                           0,0);
	databit =  gtk_combo_box_new_text();
	gtk_table_attach ( GTK_TABLE(table),
                           databit,
                           1,2,3,4,
                           GTK_SHRINK, GTK_SHRINK,
                           0,0);
	gtk_combo_box_append_text (GTK_COMBO_BOX(databit),"5");
	gtk_combo_box_append_text (GTK_COMBO_BOX(databit),"6");
	gtk_combo_box_append_text (GTK_COMBO_BOX(databit),"7");
	gtk_combo_box_append_text (GTK_COMBO_BOX(databit),"8");
	gtk_combo_box_set_active (GTK_COMBO_BOX(databit), 3);
	
	label = gtk_label_new("停止位：");
	gtk_table_attach ( GTK_TABLE(table),
                           label,
                           0,1,4,5,
                           GTK_SHRINK, GTK_SHRINK,
                           0,0);
	stopbit =  gtk_combo_box_new_text();
	gtk_table_attach ( GTK_TABLE(table),
                           stopbit,
                           1,2,4,5,
                           GTK_SHRINK, GTK_SHRINK,
                           0,0);
	gtk_combo_box_append_text (GTK_COMBO_BOX(stopbit),"1");
	gtk_combo_box_append_text (GTK_COMBO_BOX(stopbit),"2");
	gtk_combo_box_set_active (GTK_COMBO_BOX(stopbit), 0);
	
	label = gtk_label_new("流控：");
	gtk_table_attach ( GTK_TABLE(table),
                           label,
                           0,1,5,6,
                           GTK_SHRINK, GTK_SHRINK,
                           0,0);
	flowcontrol =  gtk_combo_box_new_text();
	gtk_table_attach ( GTK_TABLE(table),
                           flowcontrol,
                           1,2,5,6,
                           GTK_SHRINK, GTK_SHRINK,
                           0,0);
	gtk_combo_box_append_text (GTK_COMBO_BOX(flowcontrol),"none");
	gtk_combo_box_append_text (GTK_COMBO_BOX(flowcontrol),"RTS/CTS");
	gtk_combo_box_append_text (GTK_COMBO_BOX(flowcontrol),"Xon/Xoff");
	gtk_combo_box_set_active (GTK_COMBO_BOX(flowcontrol), 0);
	
//	gtk_widget_show(serial_number);
	gtk_widget_show_all(serialconfig_dialog);
	//gtk_builder_connect_signals (serialdailog_builder, NULL); 
//	g_object_unref (G_OBJECT (serialdailog_builder));
	
}


gchar *strerror_utf8(int errornum)
{   
	gchar *utf8error;  
	utf8error = g_locale_to_utf8((gchar *)(strerror(errornum)), -1, NULL, NULL, NULL);
	return utf8error;
} 


void ok_button_clicked(GtkButton *button, gpointer data)
{

	//GtkBuilder *serialdailog_builder = GTK_BUILDER(data);
	//GtkWidget  *serialconfig_dialog = GTK_WIDGET (gtk_builder_get_object (serialdailog_builder, "serialconfig_dialog"));
	gchar  *str_err;
	gboolean  serial_open_status;
	GtkWidget  *serial_open_err_dialog;
	GtkWidget  *serialconfig_dialog = GTK_WIDGET(data);

	device_name = gtk_combo_box_get_active_text(GTK_COMBO_BOX(serial_number));
	baud_num = gtk_combo_box_get_active(GTK_COMBO_BOX(bitrate));
	veri_num = gtk_combo_box_get_active(GTK_COMBO_BOX(checkbit));
	data_num = gtk_combo_box_get_active(GTK_COMBO_BOX(databit));
	stop_num = gtk_combo_box_get_active(GTK_COMBO_BOX(stopbit));

    serial_thread_check();	

	serial_open_status = serial_init();
	
	//g_object_unref (G_OBJECT (serialdailog_builder));
	gtk_widget_destroy(serialconfig_dialog);
	
	if(serial_open_status==FALSE)
	{
		str_err = g_strdup_printf(("Cannot open %s : %s"), device_name, strerror_utf8(errno));
		serial_open_err_dialog = gtk_message_dialog_new (GTK_WINDOW(mainwindow),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					str_err);
		//"打开串口失败！\n请检查串口是否被占用或是否有权限打开设备！"
		gtk_dialog_run (GTK_DIALOG(serial_open_err_dialog));
		gtk_widget_destroy(serial_open_err_dialog);
		g_free(str_err);
		
	}
	
}

void cancel_button_clicked(GtkButton *button, gpointer data)
{
	//GtkBuilder *serialdailog_builder = GTK_BUILDER(data);
	//GtkWidget  *serialconfig_dialog = GTK_WIDGET (gtk_builder_get_object (serialdailog_builder, "serialconfig_dialog"));
	GtkWidget  *serialconfig_dialog = GTK_WIDGET(data);
	
	//g_object_unref (G_OBJECT (serialdailog_builder));
	gtk_widget_destroy(serialconfig_dialog);
}


gboolean serial_init(void)
{
	char title[64];
	if(set_serial_init(device_name,baud_num,veri_num,data_num,stop_num)==FALSE)
	{
		set_status_message("open serial error");
		g_print("open serial error\n");
		return FALSE;
	}
	else
	{
		g_print("选择串口%s\n",device_name);
		set_status_message(device_name);
		sprintf(title,"gtkminicom-%s", device_name);
		gtk_window_set_title (GTK_WINDOW(mainwindow), title);
		sprintf(title,"Gtkminicom-%s", device_name);
		gtk_status_icon_set_tooltip (trayIcon, title);
		serial_read_thread_create();
		gtk_widget_set_sensitive (GTK_WIDGET(toolbarwidget.connect), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(toolbarwidget.disconnect), TRUE);
		return TRUE;
	}
	
}



