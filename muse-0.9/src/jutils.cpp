/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2002 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <libintl.h>

#include <jutils.h>
#include <gui.h>
#include <config.h>



static int verbosity;
static FILE *logfd = NULL;
static GUI *gui = NULL;

void set_guimsg(GUI *g) {
  gui = g;
}

void MuseSetDebug(int lev) {
  lev = lev<0 ? 0 : lev;
  lev = lev>3 ? 3 : lev;
  verbosity = lev;
}

int MuseGetDebug() {
  return(verbosity);
}


void notice(const char *format, ...) {
  char msg[255];
  va_list arg;
  va_start(arg, format);

  vsnprintf(msg, 254, format, arg);

  if(logfd) { fputs(msg,logfd); fputs("\n",logfd); fflush(logfd); }
  else fprintf(stderr,"[*] %s\n",msg);

  if(gui) gui->set_status(msg);

  va_end(arg);
}

void func(const char *format, ...) {
  if(verbosity>=2) {
    char msg[255];
    va_list arg;
    va_start(arg, format);
    
    vsnprintf(msg, 254, format, arg);
    if(logfd) { fputs(msg,logfd); fputs("\n",logfd); fflush(logfd); }
    else fprintf(stderr,"[F] %s\n",msg);

    va_end(arg);
  }
}

void error(const char *format, ...) {
    char msg[255];
    va_list arg;
    va_start(arg, format);
    
    vsnprintf(msg, 254, format, arg);
    if(logfd) { fputs(msg,logfd); fputs("\n",logfd); fflush(logfd); }
    else {
      fprintf(stderr,"[!] %s\n",msg);
      if(errno)
	fprintf(stderr,"[!] %s\n",strerror(errno));
    }
    
    if(gui) gui->set_status(msg);

    va_end(arg);
}

void act(const char *format, ...) {
  char msg[255];
  va_list arg;
  va_start(arg, format);
  
  vsprintf(msg, format, arg);

  if(logfd) { fputs(msg,logfd); fputs("\n",logfd); fflush(logfd); }
  else fprintf(stderr," .  %s\n",msg);
  
  va_end(arg);
}

void warning(const char *format, ...) {
  if(verbosity>=1) {
    char msg[255];
    va_list arg;
    va_start(arg, format);
    
    vsprintf(msg, format, arg);

    if(logfd) { fputs(msg,logfd); fputs("\n",logfd); fflush(logfd); }
    else fprintf(stderr,"[W] %s\n",msg);
  
    va_end(arg);
  }
}

void MuseSetLog(char *file) {
  logfd = fopen(file,"w");
  if(!logfd) {
    error("can't open logfile %s",file);
    error("%s",strerror(errno));
  }
}

void MuseCloseLog() {
  if(logfd) fclose(logfd);
}


void jsleep(int sec, long nsec) {
  int ret;
  struct timespec timelap;
  timelap.tv_sec = sec;
  timelap.tv_nsec = nsec;
  do {ret = nanosleep(&timelap,NULL);} while (ret==-1 && errno==EINTR);
}

double dtime() {
  struct timeval mytv;
  gettimeofday(&mytv,NULL);
  return((double)mytv.tv_sec+1.0e-6*(double)mytv.tv_usec);
}

void chomp(char *str) {
  size_t len; //, ilen;
  char tmp[MAX_PATH_SIZE], *p = str;
  
  memset(tmp,'\0',MAX_PATH_SIZE);
  
  /* eliminate space and tabs at the beginning */
  while (*p == ' ' || *p == '\t') p++;
  strncpy(tmp, p, MAX_PATH_SIZE);
  
  /* point *p at the end of string */
  len = strlen(tmp); 
  p = &tmp[len-1];
  
  while ((*p == ' ' || *p == '\t' || *p == '\n') && len) {
    *p = '\0'; p--; len--;
  }

  strncpy(str, tmp, MAX_PATH_SIZE);
}

