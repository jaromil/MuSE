/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2003 Denis Rojo aka jaromil <jaromil@dyne.org>
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
 *
 * "$Id$"
 *
 */

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <generic.h>
#include <outchannels.h>
#include <jutils.h>
#include <config.h>

#ifdef HAVE_LAME
#include <out_lame.h>
#endif

#ifdef HAVE_VORBIS
#include <out_vorbis.h>
#endif


/* I think a tighter bound could be:  (mt, March 2000)
 * MPEG1:
 *    num_samples*(bitrate/8)/samplerate + 4*1152*(bitrate/8)/samplerate + 512
 * MPEG2:
 *    num_samples*(bitrate/8)/samplerate + 4*576*(bitrate/8)/samplerate + 256
 */

/* mp3 encoder */


OutChannel::OutChannel(char *myname)
  : Entry() {
  func("OutChannel::OutChannel(%s) %p",myname,this);

  quit = false;
  initialized = false;
  running = false;
  sprintf(name,"%s",myname);
  sprintf(version, "  ");
  encoded = 0;
  fd = NULL;
  _thread_initialized = false;
  _thread_init();

  idseed = 0;

  erbapipa = new Pipe(OUT_PIPESIZE);
  erbapipa->set_block(true,true);
  erbapipa->set_block_timeout(500,500);

  /* setup defaults */
  quality(4.0);
  bps(24);
  freq(22050);
  channels(1);
  lowpass(0);
  highpass(0);
    
  //  profile_changed = true;

}

OutChannel::~OutChannel() {
  func("OutChannel::~OutChannel");

  
  quit = true;
  unlock();// signal();  

  initialized = false;

  jsleep(0,50);

  Shouter *ice = (Shouter*)icelist.begin();
  lock_ice();
  while(ice) {
    icelist.rem(1);
    delete ice;
    ice = (Shouter*)icelist.begin();
  }
  unlock_ice();

  delete erbapipa;

  if(fd) dump_stop();


  /* QUAAAA */
  //_thread_destroy();
}

void OutChannel::start() {
  Shouter *ice = (Shouter*) icelist.begin();
  lock_ice();
  while(ice) {
    if( ice->apply_profile() )
      ice->start();
    ice = (Shouter*) ice->next;
  }
  unlock_ice();
  pthread_create(&_thread, &_attr, &kickoff, this);
}

void OutChannel::run() {
  int res;
  /*
  if(!initialized) {
    warning("OutChannel::run() : output channel uninitialized, thread won't start");
    return;
  }
  */

  running = true;
  while(!quit) {

    /* check if we must encode */
    encoding = false;
    if(fd) encoding = true;
    Shouter *ice = (Shouter*)icelist.begin();
    while(ice) {
      if(ice->running) encoding = true;
      ice = (Shouter*)ice->next;
    }
    if(!initialized) encoding = false;
    
    if(!encoding) {
      jsleep(0,50);
      shout(); /* in case there are waiting retries */
      if(quit) break;
      continue;
    } else jsleep(0,5); /* avoid a tight loop */

    /* erbapipa sucking is now done in instantiated classes
       inside the encode() method
       (see vorbis and lame classes)
    */

    encoded = 0;

    lock();
    encode();
    unlock();

    if(encoded<1) continue;
    

    calc_bitrate(encoded);
    /* stream it to the net */
    res = shout();

    /* save it on the harddisk */
    res = dump();

    /* TODO: flush when erbapipa->read != OUT_CHUNK */
    
  }

  running = false;
}

int OutChannel::create_ice() {
  Shouter *ice = new Shouter();

  if(!ice) {
    error("can't create icecast shouter");
    return -1;
  }

  /* the icecast id is the position in the linklist */
  lock_ice();
  icelist.add(ice);
  idseed++;
  ice->id = id + idseed; 


  switch(tipo) {

  case MP3:
    ice->format = SHOUT_FORMAT_MP3;
    ice->login(SHOUT_PROTOCOL_HTTP);
    break;

  case OGG:
    ice->format = SHOUT_FORMAT_VORBIS;
    ice->login(SHOUT_PROTOCOL_HTTP);
    break;

  default:
    error("codec is not streamable");
    delete ice;
    return -1;
  }

  ice->apply_profile();

  unlock_ice();
  func("outchannel id %i creates new shouter %p with id %i",id,ice,ice->id);
  return ice->id;
}

bool OutChannel::delete_ice(int iceid) {

  Shouter *ice = get_ice(iceid);
  if(!ice) {
    warning("OutChannel::delete_ice(%i) : invalid id",iceid);
    return false;
  }

  if(ice) {
    lock_ice();
    ice->rem();
    if(ice->running) ice->stop();
    delete ice;
    unlock_ice();
  }

  func("outchannel id %i deleted shouter id %i",id,iceid);
  return true;
}

Shouter *OutChannel::get_ice(int iceid) {
  return (Shouter*)icelist.pick_id(iceid);
}

bool OutChannel::apply_ice(int iceid) {
  bool res = false;
  
  Shouter *ice = get_ice(iceid);

  if(ice) {
    lock_ice();
    res = ice->apply_profile();
    unlock_ice();
  }
  return res;
}

bool OutChannel::connect_ice(int iceid, bool on) {
  bool res = false;
  func("OutChannel::connect_ice(%i,%i)",iceid,on);
  Shouter *ice = get_ice(iceid);
  if(!ice) { 
    error("Outchannel::connect_ice : can't find shouter with id %i",iceid);
    return false;
  }

  lock_ice();
  res = (on) ? ice->start() : ice->stop();
  unlock_ice();
  
  return res;
}

int OutChannel::shout() {
  int res, sentout = 0;
  time_t now = time(NULL);
  lock_ice();
  Shouter *ice = (Shouter*)icelist.begin();
  while(ice) {
    if(ice->running) {
      res = ice->send(buffer,encoded);
      if(res<0) { /* there is an error: -1=temporary , -2=fatal */
	if(res==-1) { ice = (Shouter*)ice->next; continue; }
	if(res==-2) {
	  error("fatal error on stream to %s:%u",ice->host(),ice->port());
	  ice->stop();
	  notice("retrying to connect to %s:%u%s after %i seconds",
		 ice->host(), ice->port(), ice->mount(), RETRY_DELAY);
	  ice->retry = now;
	  //	  ice = (Shouter*)ice->next;
	  //	  continue;
	}
      } else sentout += res;
    } else if(ice->retry>0) {
      if((now - ice->retry) > RETRY_DELAY) {
	notice("try to reconnect to %s:%u%s",
	       ice->host(), ice->port(), ice->mount());
	if( ice->start() ) ice->retry = 0;
	else ice->retry = now;
      }
    }
    ice = (Shouter*)ice->next;
  }
  unlock_ice();
  return sentout;
}

bool OutChannel::dump_start(char *file) {
  struct stat st;
  char temp[MAX_PATH_SIZE];
  int num = 0;
  
  if(fd) {
    warning("%s channel allready dumping to %s",name,fd_name);
    return false;
  }

  /* avoid to overwrite existent files */
  snprintf(temp,MAX_PATH_SIZE,"%s",file);
  while(stat(temp, &st) != -1) {
    /* file EXIST */
    num++;
    snprintf(temp,MAX_PATH_SIZE,"%s.%i",file,num);
  }

  fd = fopen(temp,"wb"); /* writeonly nonblocking binary (-rw-rw-r--) */
  if(!fd) {
    error("%s channel can't open %s for writing",name,temp);
    act("%s",strerror(errno));
    return(false);
  }

  strncpy(fd_name,temp,MAX_PATH_SIZE);
  notice("%s channel dumping to file %s",name,fd_name);

  return true;
}

bool OutChannel::dump_stop() {
  func("OutChanne::dump_stop()");
  if(!fd) {
    warning("%s channel is not dumping to any file",name);
    return false;
  }

  fflush(fd);
  
  act("%s channel stops dumping to %s",name,fd_name);
  
  fclose(fd);
  fd = NULL;

  return true;
}

bool OutChannel::dump() {
  int res;
  if(!fd) return false;
  func("OutChannel::dump() encoded %i",encoded);
  fflush(fd);
  if(!encoded) return true;
  res = fwrite(buffer,1,encoded,fd);
  if(res != encoded)
    warning("skipped %u bytes dumping to file %s",encoded - res,fd_name);
  return true;
}
/*
void OutChannel::bps(int in) {
  _bps = in;
  Shouter *ice = (Shouter*)icelist.begin();
  while(ice) {
    ice->_bps = in;
    ice = (Shouter*)ice->next;
  }
}
*/
bool OutChannel::calc_bitrate(int enc) {
  /* calcolates bitrate */
  bytes_accu += enc;
  now = dtime();
  if((now-prev)>1) { /* if one second passed */
    bitrate = (bytes_accu<<2);
    bytes_accu = 0;
    prev = now;
    return true;
  }
  return false;
}

char *OutChannel::quality(float in) {
  int q = (int)fabs(in);
  _quality = in;

  if(!in) {
    snprintf(quality_desc,256,"%uKbit/s %uHz",bps(),freq());
    return quality_desc;
  }

  //  if(channels()<1) channels(1);
  //  if(channels()>2) channels(2);

  switch(q) {

#define BPSVAL(b,f) \
bps(b); freq(f); \
snprintf(quality_desc,256,"%uKbit/s %uHz",bps(),freq());
//if(bps()==0) bps(b); if(freq()==0) freq(f);
  
  case 0: BPSVAL(8,11025); break;
  case 1: BPSVAL(16,16000); break;
  case 2: BPSVAL(16,22050); break;
  case 3: BPSVAL(24,16000); break;
  case 4: BPSVAL(24,22050); break;
  case 5: BPSVAL(48,22050); break;
  case 6: BPSVAL(56,22050); break;
  case 7: BPSVAL(64,44100); break;
  case 8: BPSVAL(96,44100); break;
  case 9: BPSVAL(128,44100); break;

  }
  return quality_desc;
}

void OutChannel::push(void *data, int len) {
  int errors = 0;
  /* check out if encoders are configured */
  if(!encoding) return;
  if(!initialized) return;

  /* push in data
     wait if pipe is full or occupied
     returns the right thing */
  //  func("PID %i wants to push %i bytes in %s",getpid(),len,name);
  //  func("pipe has %i free space",erbapipa->space());
  while( running && 
	 erbapipa->write(len,data) < 0 ) {
    //    func("PID %i waits to push %i bytes in %s",getpid(),len,name);
    jsleep(0,30);
    errors++;
    if(errors>20) {
      warning("%s encoder is stuck, pipe is full",name);
      return;
    }
  }
  //  func("ok, %i bytes pushed succesfully in %s",len,name);
}

/* thread stuff */

void OutChannel::_thread_init() {
  if(_thread_initialized) return;

  func("OutChannel::thread_init()");
  if(pthread_mutex_init (&_mutex,NULL) == -1)
    error("error initializing POSIX thread mutex");
  if(pthread_mutex_init (&_mutex_ice,NULL) == -1)
    error("error initializing POSIX thread mutex");
  if(pthread_cond_init (&_cond, NULL) == -1)
    error("error initializing POSIX thread condition"); 
  if(pthread_attr_init (&_attr) == -1)
    error("error initializing POSIX thread attribute");
  
  /* set the thread as detached
     see: man pthread_attr_init(3) */
  pthread_attr_setdetachstate(&_attr,PTHREAD_CREATE_DETACHED);

  _thread_initialized = true;
}

void OutChannel::_thread_destroy() {
  if(!_thread_initialized) return;
  
  /* we signal and then we check the thread
     exited by locking the conditional */
  if(running) {
    signal();
    lock(); unlock();
  }

  if(pthread_mutex_destroy(&_mutex) == -1)
    error("error destroying POSIX thread mutex");
  if(pthread_cond_destroy(&_cond) == -1)
    error("error destroying POSIX thread condition");
  if(pthread_attr_destroy(&_attr) == -1)
    error("error destroying POSIX thread attribute");
  _thread_initialized = false;
}
