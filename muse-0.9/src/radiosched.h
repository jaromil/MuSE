/* MuSE - Multiple Streaming Engine
 * (c) Copyright 2001 Eugen Melinte <ame01@gmx.net>
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

 $Id$
 
 */
/*
  Content:
     -generic/abstract Basic_scheduler class, threaded with code 
	  ripped from the Channel class.
     -Scheduler_text class - not used anymore
     -Scheduler_xml class 
 */


/**
   @file radiosched.h 

   @desc Schedule connections to other radios (to re-broadcast their shows) or play files. 
   This file contains necessary interfaces and some concrete classes. 
*/

#ifndef __CRADIOSCHED_H
#define __CRADIOSCHED_H

#include <math.h>
#include <pthread.h>
#include <sys/time.h>

#include <inchannels.h>
#include <outchannels.h>
#include <gui.h>

#include <generic.h>


/*
 * We can have many types of schedulers: using a cron-like format, an SQL db, 
 * xml, etc.  Usage:
 * To init (like a Channel) - @see muse.cpp : 
 *    sched = new MyBasic_scheduler( sched_data );
 *    mixer->register_sched( sched );
 *    sched->lock();
 *    sched->start();
 *    sched->wait();
 *    sched->unlock();
 * To use:
 *        sched->set_schedule( ... ); //only if different from sched_data
 *        sched->play();
 *        sched->stop();
 *        sched->play(); //etc..
 *        sched->stop();
 * To kill thread:
 *    sched->quit = true;
 */
#define SCHEDFILE       "schedule.xml" //under $HOME/.muse
const char *sched_file_path(void);


class Basic_scheduler {

 public:
  Basic_scheduler( const void *sched ); 
  virtual ~Basic_scheduler();
  
  void register_mixer(Stream_mixer *mix) {mixer=mix;};

  virtual void run();
  virtual bool stop();
  virtual bool play();
  virtual void on_wakeup();
  virtual void load_schedule() { on_load_schedule();};
  /** The only func to be written for child classes.  */
  virtual void on_load_schedule() =0; 
  
  void set_schedule( const void *s ) {_schedule = s;};

  bool on;
  bool running;
  bool quit;
  Channel *channel;

  /* pthread stuff */
  void start() {
    pthread_create(&_thread, &_attr, &kickoff, this); };
  void lock() { pthread_mutex_lock(&_mutex); };
  void unlock() { pthread_mutex_unlock(&_mutex); };
  void wait() { pthread_cond_wait(&_cond,&_mutex); };
  void signal() { pthread_cond_signal(&_cond); };
  /* ------------- */

 protected:
  static void* kickoff(void *arg) { ((Basic_scheduler*) arg)->run(); return NULL; };
  
  /* Match the left string with the right number.  */
  bool match( const char *left, int right ); 
  void stop_channel(void);
  void start_channel( Url *rec ); 
  
  const void *_schedule; 
  Playlist *playlist;
  void dump(); ///< print schedule to stdout; for debugging 
  time_t prev_time;
  //schedule_record *head, *rec_ptr; /* crontab entry pointers       */
  /* Schedule record corresponding to the current playing Basic_connection */
  Url  *playing; 
  
  void stop_inner_channel(void);
  void start_inner_channel( Url *rec ); 
  // start/stop mixer channels
  Stream_mixer *mixer;
  void stop_mixer_channel( Url *rec );
  void start_mixer_channel( Url *rec ); 

 private:
  /* pthread stuff */
  void _thread_init();
  void _thread_destroy();
  pthread_t _thread;
  pthread_attr_t _attr;
  pthread_mutex_t _mutex;
  pthread_cond_t _cond;
  /* ------------- */

};


extern Basic_scheduler *rscheduler;

/*
 * Scheduler_text uses a crontab(5) like text-file format:
 * Mn  Hr Day Mth WDay MnPlay Url comment
 */
#define CRONSIZE        (8192*2)
#define TSEPARATOR       " \t"
class Scheduler_text : public Basic_scheduler {
 public:
  Scheduler_text(const void *sched) : Basic_scheduler(sched) 
      {memset(crontab, '\0', CRONSIZE);};
  virtual ~Scheduler_text() {};
  void on_load_schedule();
  
 private:
  char crontab[CRONSIZE];  /* memory for the entries       */
  void load_crontab( const char *cronf ); 
  void addentry( char *line ); 
  
};


/*
 * Schedule record chain.  The strings can contain wildcards:
 *      *               @match 1 for any number
 *      x,y [,z, ...]   @match 1 for any number given 
 *      x-y             @match 1 for any number within the range of x thru y 
 * Format might change in the future, as scheduler gets more mature. 
 * @see Url class also.
 */
typedef struct  _sched_rec {
	const char *src;
	const char *comment;
	const char *wkd;      ///< 0-6 or Mon, Tue, ...
	const char *month;    ///< 1-12 or Jan, Feb, ...
	const char *day;      ///< 1-31
	const char *stime;    ///< start time xx:xx 24-hours format
	const char *etime;    ///< end time
	const char *secs;     ///< seconds
} sched_rec;

/*
 * xml format scheduler 
 */
class Scheduler_xml : public Basic_scheduler {
 public:
  Scheduler_xml(const void *sched) : Basic_scheduler(sched) {};
  void on_load_schedule();
  virtual ~Scheduler_xml() {};
  
 private:
  void load_crontab( const char *cronf ); 
  static int addentry( void *instance, sched_rec *sr ); 
  
};


/*
 * xml schedule parser - to be used by GUIs, etc. 
 */
typedef int (*sched_rec_callb)(void*, sched_rec*); 
/** Parse $HOME/.muse/ SCHEDFILE and call @param callb for every 
    record.  @return TRUE if parse successfull. */
int parse_xml_sched_file( sched_rec_callb callb, void *udata, sched_rec *data );
/** Create an empty one if it does not exists.  */
int create_xml_schedule_file(void);
/** No xml header/footer, just content as xml tags - obtained by
    concatenation of format_xml_sched_rec().  */
int write_xml_schedule_file(const char *content); 
/** Must free() returner pointer.  */
char *format_xml_sched_rec(const sched_rec *rec);

#endif /*__CRADIOSCHED_H*/

