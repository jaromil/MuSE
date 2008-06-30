/* scheduler.cpp - MuSE

   this class is a synchronous object
   comes out a thread launched in the main() (muse.cpp)

   queue processing is done here
   commands are stored in the queue file like that:
   SECS.NANOSECS\tCMD\tARG\tARG....\n
   time it's absolute, counting 0 as the starting time
   the time value is a float
*/

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "scheduler.h"

Scheduler::Scheduler(char *queue_file, Stream_mixer *mix) {

  queuefd = fopen(queue_file, "r");
  if(queuefd!=NULL)
    endqueue = false;
  else
    endqueue = true;
  
  mixer = mix;

}

void Scheduler::run() {
  do {
    
    if(fgets(nextevent,255,queuefd)!=NULL) {
      fprintf(stderr,"SCHED: %s",nextevent);
      /* got it, let's see how long we must wait */
      p = &nextevent[0];
      while(*p!='.') p++;
      *p = '\0';
      timelap.tv_sec = atoi(nextevent);
      p++; pp = p;
      while(*p!='\t') p++;
      *p = '\0';
      timelap.tv_nsec = atol(pp);
      p++; command = p;
    } else
      endqueue = true;

    if(!endqueue) {
      fprintf(stderr,"SCHED: going to wait for %lu secs %lu nsecs\n",timelap.tv_sec, timelap.tv_nsec);
      nanosleep(&timelap,NULL);
      
      fprintf(stderr,"SCHED: now it comes\n");

      /* executes the command */
      
      /* SET CHANNEL */
      if(strncmp(command,"301",3)==0) {

	int fnum, ch;
	float speed, vol;
	p = pp = command+4;
	while(*p!='\t') p++; *p = '\0';
	fnum = atoi(pp);
	p++; pp = p;
	while(*p!='\t') p++; *p = '\0';
	ch = atoi(pp);
	p++; pp = p;
	while(*p!='\t') p++; *p = '\0';
	speed = atof(pp);
	p++; pp = p;
	while(*p!='\n') p++; *p = '\0';
	vol = atof(pp);

	fprintf(stderr,"SCHED: got [set channel %u file %u vol %f speed %f]\n", ch, fnum, vol, speed);
	
	mixer->set_channel(ch,fnum);
	
	if(vol!=1.0) {
	  mixer->set_volume(ch,vol);
	}
	/*
	  if(speed!=1.0) {
	  synchronous.select.call(0, mixer, Stream_mixer::id_stream_mixer);
	  synchronous.wait();
	  mixer->set_speed(ch,speed);
	  }
	*/
      }
      /* -------- */
      
      /* STOP CHANNEL */
      if(strncmp(command,"302",3)==0) {
	int ch;
	p = pp = command+4;
	while(*p!='\n') p++; *p = '\0';
	ch = atoi(pp);
	
	mixer->stop_channel(ch);
      }
      
      /* PAUSE CHANNEL */
      if(strncmp(command,"303",3)==0) {
	int ch;
	p = pp = command+4;
	while(*p!='\n') p++; *p = '\0';
	ch = atoi(pp);
	
	mixer->pause_channel(ch);
      }
      
      /* START CHANNEL */
      if(strncmp(command,"304",3)==0) {
	int ch;
	p = pp = command+4;
	while(*p!='\n') p++; *p = '\0';
	ch = atoi(pp);
	
	mixer->play_channel(ch);
      }
      
      /* SET VOLUME */
      if(strncmp(command,"306",3)==0) {
	int ch;
	float vol;
	p = pp = command+4;
	while(*p!='\t') p++; *p = '\0';
	ch = atoi(pp);
	p++; pp = p;
	while(*p!='\n') p++; *p = '\0';
	vol = atof(pp);
	
	mixer->set_volume(ch,vol);
      }
    } /* if(!endqueue) */
  } while(!endqueue);
}

Scheduler::~Scheduler() {
  fclose(queuefd);
}
