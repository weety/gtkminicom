#ifndef __LOGFILE_H__
#define __LOGFILE_H__
#include <stdio.h>

//extern FILE *logfilefd;
char filepath[128];
void  logfile_config(void);
void logfile_refresh(void);
void logfile_close(void);
int clear_logfile(void);

#endif
