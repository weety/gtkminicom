#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <glib.h>

#include "logfile.h"
#include "serial.h"

//#define FILENAME  gtkminicom.log
static FILE *logfilefp;
static pthread_mutex_t  mutex;

void logfile_write_data(char *buf, long length);

void  get_time(char   *sTime, int length)
{
	time_t  ct;
	struct tm  *currtime;
	time(&ct);
	currtime  = (struct tm *)localtime(&ct);
	memset(sTime,0x00,length);
	sprintf(sTime, "%04d-%02d-%02d.%02d:%02d:%02d", (currtime->tm_year+1900), (currtime->tm_mon+1), currtime->tm_mday, currtime->tm_hour, currtime->tm_min, currtime->tm_sec);
	return;
}


void  logfile_config(void)
{
	char *home;
	//char filepath[128];
	char time[32];
	GString *gfilepath;
	struct stat configdir_stat;

	home = getenv("HOME");
	printf("home=%s\n",home);
	gfilepath = g_string_new(home);
	g_string_append(gfilepath, "/.gtkminicom");
	if(stat(gfilepath->str, &configdir_stat) != 0)
	{
		mkdir(gfilepath->str, 0777);
	}
	get_time(time, sizeof(time));
	g_string_append_printf(gfilepath, "/gtkminicom%s.log", time);
	strcpy(filepath, gfilepath->str);
	g_string_free(gfilepath, TRUE);
	//sprintf(filepath, "%s/gtkminicom.%s.log", home, time);
	//sprintf(filepath, "/tmp/gtkminicom%s.log", time);
	printf("%s\n",filepath);
	logfilefp = fopen(filepath, "w");  //记录日志文件
	//logfilefp = fopen(filepath, "a");  //记录日志文件
	fprintf(logfilefp, "\n\n");
	fprintf(logfilefp, "/********************************************/\n");
	fprintf(logfilefp, "/******     %s     ******/\n", time);
	fprintf(logfilefp, "/********************************************/\n");
	fprintf(logfilefp, "\n\n");
	fflush(logfilefp);
	pthread_mutex_init(&mutex, NULL);
	register_callback(logfile_write_data, "logfile");
}

int clear_logfile(void)
{
	int err;
	char time[32];
	
	pthread_mutex_lock(&mutex);
	fflush(logfilefp);	//在文件清空之前必须刷新缓存，否则会出现文件空洞
	err = ftruncate(fileno(logfilefp), 0);
	if (err)
	{
		pthread_mutex_unlock(&mutex);
		return err;
	}
	//fseek(logfilefp, 0L, SEEK_SET);
	rewind(logfilefp);
	get_time(time, sizeof(time));
	fprintf(logfilefp, "\n\n");
	fprintf(logfilefp, "/********************************************/\n");
	fprintf(logfilefp, "/******     %s     ******/\n", time);
	fprintf(logfilefp, "/********************************************/\n");
	fprintf(logfilefp, "\n\n");
	pthread_mutex_unlock(&mutex);
	fflush(logfilefp);
	return 0;
}

void logfile_refresh(void)
{
    fflush(logfilefp);
}

void logfile_close(void)
{
	pthread_mutex_destroy(&mutex);
	fclose(logfilefp);
}


void logfile_write_data(char *buf, long length)
{
	pthread_mutex_lock(&mutex);
	fwrite(buf, sizeof(char), length, logfilefp);
	pthread_mutex_unlock(&mutex);
}
