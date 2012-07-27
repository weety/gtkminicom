#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/signal.h> 
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

//#include <gtk/gtk.h>
//#include <vte/vte.h>
#include "serial.h"
#include "logfile.h"
#include "list.h"

#define G_THREAD_FUNC(f) ((GThreadFunc) (f))
#define _POSIX_SOURCE 1 /* POSIX compliant source */  
static GThread *serial_read_thread = NULL;

static gchar buf[1024];

static gint serial_fd = -1;
//struct termios oldtio;
static struct termios options;
static volatile int thread_should_exit = FALSE;

#define NAME_LEN    32
struct callback_func_list
{
	struct list_head    list;
	void (*callback)(gchar *text, glong length);
	gchar name[NAME_LEN];
};

static struct list_head callback_head= LIST_HEAD_INIT(callback_head);


gboolean OpenDev(gchar *Dev)
{
	serial_fd = open( Dev, O_RDWR );
	//| O_NOCTTY | O_NDELAY|O_NONBLOCK
	if (-1 == serial_fd)
	{
		perror("Can't Open Serial Port");
		return FALSE;
	}
    //fcntl(serial_fd, F_SETFL, 0);   //恢复串口的状态为阻塞状态，用于等待串口数据的读入//通过测试，发现无效
    //fcntl(fd, F_SETFL, FNDELAY);    //设置串口为非阻塞方式
	if(isatty(STDIN_FILENO)==0)
		printf("standard input is not a terminal device\n");
	else
		printf("isatty success!\n");

	if ( tcgetattr( serial_fd, &options) != 0)
	{
		perror("SetupSerial 1");
		return(FALSE);
 
	}
	//options.c_lflag = ICANON; 
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//不经处理直接发送
	options.c_oflag &=~(INLCR|IGNCR|ICRNL);
	options.c_oflag &=~(ONLCR|OCRNL);
	options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);////////重要
	// options.c_cc[VMIN]=1;  
//	options.c_cc[VMIN]=0;    
	// options.c_cc[VTIME]=0;
//	options.c_cc[VTIME]=2;
	tcflush(serial_fd, TCIFLUSH);  

	return TRUE;
}

gboolean set_databits(gint databits)
{
	options.c_cflag &= ~CSIZE;
	switch (databits) /*设置数据位数*/
	{  
	case 5:
		options.c_cflag |= CS5;
		break;
	case 6:
		options.c_cflag |= CS6;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr,"Unsupported data size "); 
		return (FALSE);
	}
	return (TRUE);
}

gboolean set_parity(gchar parity)
{
	switch (parity)
	{
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB; /* Clear parity enable */
		options.c_iflag &= ~INPCK; /* Enable parity checking */
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
		options.c_iflag |= INPCK; /* Disnable 漀?朦??????????parity checking */
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB; /* Enable parity */
		options.c_cflag &= ~PARODD; /* 转换为偶效验*/
		options.c_iflag |= INPCK; /* Disnable parity checking */
		break;
	case 'm':
	case 'M':
		options.c_cflag &= ~PARENB;
		options.c_cflag |= CMSPAR;
	case 's':
	case 'S': /*as no parity*/
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CMSPAR;
		break;
	default:
		fprintf(stderr,"Unsupported parity ");
		return (FALSE);
	}
  return (TRUE);
}

gboolean set_stopbits(gint stopbits)
{
	/* 设置停止位*/
 	switch (stopbits)
	{
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported stop bits ");
		return (FALSE);
	}
	return (TRUE);
}

gboolean set_speed(gint speed)
{
	gint status;
	tcflush(serial_fd, TCIOFLUSH);
	cfsetispeed(&options, speed);
	cfsetospeed(&options, speed);
	status = tcsetattr(serial_fd, TCSANOW, &options);
	if (status != 0)
	{
		perror("tcsetattr serial_fd");
		return (FALSE);
	}
	tcflush(serial_fd,TCIOFLUSH);
	return (TRUE);
}


gboolean set_term(gint databits,gint stopbits,gchar parity,gint bps)
{
	//options.c_cflag &= ~CSIZE;
	if(set_databits(databits) == FALSE)
		return (FALSE);
	if(set_parity(parity) == FALSE)
		return (FALSE);
	if(set_stopbits(stopbits) == FALSE)
		return (FALSE);
	if(set_speed(bps) == FALSE)
		return (FALSE);
	/* Set input parity option */
	// if (parity != 'n')
	// options.c_iflag |= INPCK;
 	tcflush(serial_fd,TCIFLUSH);
	if (tcsetattr(serial_fd,TCSANOW,&options) != 0)
	{
		perror("SetupSerial 3");
		return (FALSE);
	}
	return (TRUE);
}



gboolean set_serial_init(gchar *device,gint baud_num,gint veri_num,gint data_num,gint stop_num)
{
	gint speed_arr[] = { B300, B600, B1200, B2400, B4800, B9600, B19200, B38400,B57600,B115200 };
	gchar verify_arr[] = {'N','O','E','M','S'};
	gint data_bit[] = {5,6,7,8};
	gint stop_bit[] = {1,2};

	if(OpenDev(device) == FALSE)
		return (FALSE);
	if(set_term(data_bit[data_num],
			stop_bit[stop_num],
			verify_arr[veri_num],
			speed_arr[baud_num]) == FALSE)
	{
		g_print("set the serial error!\n");
		return (FALSE);
	}
	return (TRUE);
 
}


void send_serial(gchar *text, guint length)
{
	int num;
	//printf("write serial\n");
	if(serial_fd > 0)
	{
		num = write(serial_fd, text, length);
		if(num <= 0)
		{
			printf("write serial error\n");
		}
	}
}


void close_serial()
{
	if(serial_fd>0)
	{
		close(serial_fd);
		serial_fd = -1;
		//fclose(logfilefd);
	}
	
}


void  read_thread(void)
{
	glong res, res_tmp; 
	glong   len;
	int pos;
	GString *buffer_tmp;
	gchar *in_buffer;
	gchar time[32];
	struct list_head *plist;
	struct callback_func_list *callback_node;
	struct timeval select_timeout;
	fd_set rset;
	int status = 0;
	
	select_timeout.tv_sec = 1;
	select_timeout.tv_usec = 0;

	while (1)
	{       
		usleep(1000);  
		//gdk_threads_enter();
		if (thread_should_exit == TRUE)
		{
			printf("线程退出\n");
			g_thread_exit(NULL);
		}

		FD_ZERO(&rset);
		FD_SET(serial_fd, &rset);
		status = select(serial_fd + 1, &rset, NULL, NULL, &select_timeout);
		if(status == 0)
		{
			continue;
		}
		else if(status < 0)
		{
			perror("select error\n");
			continue;
		}
		
		if (FD_ISSET(serial_fd, &rset))
		{         
			ioctl(serial_fd, FIONREAD, &len);
			if (len > 1024) 
			{
				printf("len=%ld\n", len);
				len = 1024;
			}
			get_time(time, sizeof(time));
			res = read(serial_fd,buf,len); 
			if (res <= 0)
			{
				continue;
			}
			//fwrite(buf, 1, res, file); 
		//	buffer_tmp =  g_string_new(buf);
			list_for_each(plist, &callback_head)
			{
				callback_node = list_entry(plist, struct callback_func_list, list);
				if (strcmp(callback_node->name, "logfile")==0)
				{
					buffer_tmp =  g_string_new_len(buf, res);
					res_tmp = res;
					//in_buffer = buffer_tmp->str;
					/*过滤非文本数据*/
					for(pos=0; pos < res_tmp; pos++)
					{
						in_buffer = buffer_tmp->str + pos;
						if((*in_buffer > 0x7D) || ((*in_buffer < 0x20) && (*in_buffer != 0x0A) && (*in_buffer != 0x0D)))
						{
							//*in_buffer = ' ';
							buffer_tmp = g_string_overwrite_len(buffer_tmp, pos, " ", 1);
						}
						//in_buffer++;
						if (*in_buffer == 0x0A)
						{
							buffer_tmp = g_string_insert(buffer_tmp, pos+1, time);
							pos += strlen(time);
							buffer_tmp = g_string_insert_len(buffer_tmp, pos+1, "  ", 2);
							pos += 2;
							res_tmp = buffer_tmp->len;
						}
					}
					callback_node->callback(buffer_tmp->str, res_tmp);
					g_string_free (buffer_tmp, TRUE);
				}
				else
				{
					callback_node->callback(buf, res);
				}
			}

		}   
		//gdk_threads_leave();   
	}       
}


//extern VteTerminal *display;
/*创建串口读线程*/
void serial_read_thread_create(void)
{
	thread_should_exit = FALSE;
	serial_read_thread = g_thread_create(G_THREAD_FUNC(read_thread),NULL,TRUE,NULL);//创建线程
	//serial_read_thread = g_thread_create(G_THREAD_FUNC(read_thread),display,TRUE,NULL);//创建线程
	
}

/*串口读线程退出*/
void serial_read_thread_exit(void)
{
	if(serial_read_thread!=NULL)
	{
		thread_should_exit = TRUE;
		g_thread_join(serial_read_thread);
		close_serial();
		serial_read_thread = NULL;
	}
}

/*检测是否已经有线程在运行*/
void serial_thread_check(void)
{
	if(serial_read_thread!=NULL)
	{
		serial_read_thread_exit();
	}
}

/*注册回调函数*/
int register_callback(void (*callback_func)(gchar *text, glong length), gchar *name)
{
	struct callback_func_list *callback_node = malloc(sizeof(struct callback_func_list));

	if (!callback_node)
	{
		printf("<register_callback> malloc failed\n");
		return -1;
	}

	if (strlen(name) > (NAME_LEN-1))
	{
		printf("register callback name too length\n");
		free(callback_node);
		return -1;
	}

	sprintf(callback_node->name, "%s", name);
	callback_node->callback = callback_func;
	list_add(&callback_node->list, &callback_head);

	return 0;

}

/*注销回调函数*/
void unregister_callback(gchar *name)
{
	struct list_head *pos;
	struct list_head *tmp;
	struct callback_func_list *callback_node;

	list_for_each_safe(pos, tmp, &callback_head)
	{
		callback_node = list_entry(pos, struct callback_func_list, list);
		if (strcmp(callback_node->name, name)==0)
		{
			list_del(pos);
			free(callback_node);
		}
	}
	
}

/*注销所有回调函数*/
void unregister_all_callback(void)
{
	struct list_head *pos;
	struct list_head *tmp;
	struct callback_func_list *callback_node;

	list_for_each_safe(pos, tmp, &callback_head)
	{
		callback_node = list_entry(pos, struct callback_func_list, list);
		list_del(&callback_node->list);
		free(callback_node);
	}
}
