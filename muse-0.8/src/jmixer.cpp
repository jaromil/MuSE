/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2003 Denis Roio aka jaromil <jaromil@dyne.org>
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

 "$Id$"
 
 */

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/time.h>

#include <jutils.h>
#include <audioproc.h>
#include <jmixer.h>

#include <in_oggvorbis.h>
#include <in_mpeg.h>

#include <config.h>

#ifdef HAVE_VORBIS
#include <out_vorbis.h>
#endif

#ifdef HAVE_LAME
#include <out_lame.h>
#endif

#define CODENAME "COTURNIX"

/* process_buffer BUF_SIZE is:
   BUF_SIZE of 32bit ints *2channels *8resampling_space

   audio_buffer BUF_SIZE is:
   BUF_SIZE of 16bit short *2channels *8resampling_space
*/

Stream_mixer::Stream_mixer() {
  int i;
  for(i=0;i<MAX_CHANNELS;i++)
    chan[i] = NULL;

  /* here memset takes byte num */
  memset(process_buffer,0,PROCBUF_SIZE*sizeof(int32_t));
  memset(audio_buffer,0,PROCBUF_SIZE*sizeof(int16_t));

  dsp = 0;
  max = 0;
  have_gui = false;

  dspout = false;
  linein = false;
  fileout = false;
  quit = false;

  for(i=0;i<8;i++) peak[i] = 0;
  cpeak = 0;

  /* this is the base seed for new encoders id */
  idseed = 0; //abs(time(NULL) & getpid()) >> 2;

  if(pthread_mutex_init (&_mutex,NULL) == -1)
    error("error initializing POSIX thread mutex");
  if(pthread_cond_init (&_cond, NULL) == -1)
    error("error initializing POSIX thread condtition"); 
  unlock();
}

Stream_mixer::~Stream_mixer() {
  quit = true;
  func("Stream_mixer::~Stream_mixer()");
  int i;
  
  if(dsp>0) {
    act("closing soundcard");
    close_soundcard();
  }

  act("deleting input channels");
  for(i=0;i<MAX_CHANNELS;i++) {
    /* delete_channel(i); */
    if(chan[i]) delete_channel(i);
  }

  act("deleting output channels");
  OutChannel *outch = (OutChannel*) outchans.begin();
  while(outch) {
    delete_enc( outch->id );
    outch = (OutChannel*) outchans.begin();
  }

  func("deleting thread mutexes");
  if(pthread_mutex_destroy(&_mutex) == -1)
    error("error destroying POSIX thread mutex");
  if(pthread_cond_destroy(&_cond) == -1)
    error("error destroying POSIX thread condition");

}

void Stream_mixer::register_gui(GUI *reg_gui) { 
  char temp[256];  
  gui = reg_gui; 
  have_gui = true;
  sprintf(temp,"%s %s codename \"%s\"",PACKAGE, VERSION, CODENAME);
  gui->set_title(temp);
}

bool Stream_mixer::open_soundcard() {
  int format,tstereo,speed,caps; //,val;   
  
  /* can't be nonblocking for correct playing
     but to check if /dev/dsp is free we must use that */
  if((dsp=open("/dev/dsp",O_RDWR|O_NONBLOCK))==-1) {
    error("can't open soundcard: %s", strerror(errno));
    return(false);
  } else {
    if(fcntl(dsp, F_SETFL, 0) <0) { /* remove O_NONBLOCK */
      close(dsp);
      error("can't switch to blocking mode");
      return(false);
    }
    notice("Found soundcard on /dev/dsp");
  }
  
  /* BUFFER FRAGMENTATION
     val = (FRAGCOUNT << 16) | FRAGSIZE;  
     if(ioctl(dsp,SNDCTL_DSP_SETFRAGMENT,&val)==-1)
     error("failed to set dsp buffers:");
  */


  format = AFMT_S16_LE;
  
  tstereo = 1; /* only stereo _DSP_ in/out */
  speed = SAMPLE_RATE; /* 44100hz mixing */
  
  ioctl(dsp, SNDCTL_DSP_GETCAPS, &caps);
  if(caps & DSP_CAP_DUPLEX) {
    fullduplex = true;
    act("full duplex supported. good");
    ioctl(dsp, SNDCTL_DSP_SETDUPLEX, 0);
    ioctl(dsp, DSP_CAP_DUPLEX, 0);
  } else {
    act("only halfduplex is supported");
    fullduplex = false;
  }
  
  /* FORMAT
     if(ioctl(dsp,SNDCTL_DSP_SETFMT,&format)==-1)
     error("failed to set data format");
  */
  if(ioctl(dsp,SNDCTL_DSP_SAMPLESIZE,&format) <0)
    error("failed to set dsp samplesize");
  
  /* CHANNELS */
  if(ioctl(dsp,SNDCTL_DSP_STEREO,&tstereo)==-1)
    error("something went wrong with the stereo setting");
  
  /* SAMPLERATE */
  if(ioctl(dsp,SNDCTL_DSP_SPEED,&speed)==-1)
    error("speed setting failed");
  
  /* GET FRAG SIZE BACK
     if(ioctl(dsp,SNDCTL_DSP_GETBLKSIZE,&val)==-1)
     error("get block size failed to return");
  */
  
  act("mixing 16bit %dHz stereo",speed);
  
  dspout = true;
  
  livein.init(speed, tstereo+1, &dsp);
  
  return(true);
} /* open_soundcard */

void Stream_mixer::cafudda()
{
  int i, c=0, cc;
  int total_bitrate=0;

  /* here memset takes byte num
     max *4 (32bit) *2 (stereo) */
  memset(process_buffer,0,MIX_CHUNK<<3);

  //  max = 0;
  peak[cpeak] = 0;

  if(quit) {
    func("QUIT detected while cafudding");
    return;
  }

  lock();

  for(i=0;i<MAX_CHANNELS;i++) {
    if(chan[i] != NULL) {
      if(chan[i]->on) {
	// OPTIMIZE: elimina l'azzeramento all'inizio e fallo nella prima passata di mix
	cc = chan[i]->erbapipa->mix16stereo(MIX_CHUNK,process_buffer);
	// if(cc!=MIX_CHUNK<<2) warning("hey! mix16stereo on ch[%u] returned %i",i,cc);
	c+=cc;
	updchan(i);
	
      } /* if(chan[i].on) */
    } /* if(chan[i] != NULL) */
  } /* for(i=0;i<MAX_CHANNELS;i++) */
  
  if(linein) {
    // ires = livein.mix(process_buffer);
    c += livein.mix(process_buffer);
    // max = (max<ires) ? ires : max;
  }
  
  /* here: max = number of 32bit samples in process_buffer
     number of single 16bit stereo samples (max<<1)
     number of bytes (max<<2)
     func("mixxing %i samples (%i bytes)",max,max<<2);
  */

  if(c>0) {
    /* CLIPPING
       this brings it back to a 16bit resolution 
       and puts it into audio_buffer    */
    clip_audio(MIX_CHUNK);
    
    unlock();

    out = (OutChannel*) outchans.begin();
    while(out) {
      if(!out->running) {
	out = (OutChannel*) out->next;
	continue;      }
      out->push(audio_buffer,MIX_CHUNK<<2);
      total_bitrate += out->get_bitrate();
      out = (OutChannel*) out->next;
    }

    /* WRITE 2 DSP */
    if(dspout) {
      /* write out interleaved stereo 16bit pcm 
	 dsp takes number of *BYTES*, the format
	 is being setted with ioctls in initialization */
      write(dsp,audio_buffer,MIX_CHUNK<<2);
    }
    
    /* compute and draw levels */
    cpeak++;
    if(have_gui 
       && cpeak==8 
       && gui->meter_shown()) {
      gui->vumeter_set( (peak[0]+peak[1]+peak[2]+peak[3]+
			 peak[4]+peak[5]+peak[6]+peak[7])>>3 );
      gui->bpsmeter_set( total_bitrate );
      cpeak = 0;
    }
    
  } else {
    
    unlock();
    
    if(have_gui) {
      if(gui->meter_shown()) {
	gui->vumeter_set( 0 );
	gui->bpsmeter_set( 0 );
      }
    }

  }

  /* notice the gui to refresh */
  if(have_gui) gui->signal();
  
  /* we don't want massive usage of the cpu
     also thread synchronization is a shamanic practice ;)
     in which we must find the right moment to breath;
     
     here we give fifos a bit of air and avoid tight loops
     making the mixing engine wait 10 nanosecs */
  jsleep(0,50);

}

bool Stream_mixer::create_channel(int ch) {
  lock();

  /* paranoia */
  if(chan[ch]!=NULL)
    delete_channel(ch);

  /* by default we create a mpeg channel */
  chan[ch] = new MpegChannel();

  unlock();
  
  if(chan[ch]==NULL)
    return(false);

  return(true);
}

void Stream_mixer::delete_channel(int ch) { 
  /* paranoia */
  if(!chan[ch]) {
    warning("Stream_mixer::delete_channel(%u) called on a NULL channel",ch);
    return;
  }

  lock();
  if(chan[ch]->on) chan[ch]->stop();
  /* quit the thread */
  if(chan[ch]->running) {
    chan[ch]->quit = true;
    /* be sure it quitted */
    chan[ch]->signal();
    chan[ch]->lock(); chan[ch]->unlock();
    jsleep(0,100);
  }

  /* clean internal allocated buffers */
  delete chan[ch];
  chan[ch] = NULL;
  playlist[ch].cleanup();
  unlock();
}

void Stream_mixer::pause_channel(int ch) {
  /* paranoia */
  if(!chan[ch]) {
    warning("Stream_mixer::pause_channel(%u) called on a NULL channel",ch);
    return;
  }

  /* here i don't lock - c'mon, boolean _is_ atomical */
  if(chan[ch]->opened) {
    if(!chan[ch]->on) {
      lock();
      if(!chan[ch]->play())
	error("can't play channel %u",ch,ch);
      unlock();
    } else {
      chan[ch]->on = false;
      chan[ch]->position = chan[ch]->time.f;
    }
  } else error("tried to switch pause on unopened channel %i",ch);
} /* overloaded non-switching function follows */
void Stream_mixer::pause_channel(int ch, bool stat) { /* if stat==true -> pause the channel */
  /* paranoia */
  if(!chan[ch]) {
    warning("Stream_mixer::pause_channel(%u) called on a NULL channel",ch);
    return;
  }
  if(chan[ch]->opened) {
    if(!stat) {
      lock();
      if(!chan[ch]->play())
	error("can't play channel %u",ch,ch);
      unlock();
    } else chan[ch]->on = false;
  } else error("can't pause unopened channel %i",ch);
}

/*
  set the active sound on the channel
  the file/stream is loaded and a channel is created to handle it
  takes the channel number and the position of the song into the playlist
  ** int pos starts from 1
  set_channel returns:
  0 - error
  1 - bitstream opened (seekable)
  2 - bitstream opened (non seekable)
*/
int Stream_mixer::set_channel(int ch, int pos) {
  int res;
  char *sel = playlist[ch].song(pos);

  func("Stream_mixer::set_channel(%i,%i)",ch,pos);

  if(!sel) {
    error("channel %u has no entry %u in playlist",ch,pos);
    return(0);
  }

  lock();

  if(strncasecmp(sel+strlen(sel)-4,".ogg",4)==0) {
#ifdef HAVE_VORBIS
    /* take care to delete the old channel if present*/
    if(chan[ch]!=NULL) {
      if(chan[ch]->type != OGGCHAN) {
	func("deleting previous MpegChannel");
	delete (OggChannel*)chan[ch];
	/* better to wait a bit after deleting */
	jsleep(0,100);
	chan[ch] = new OggChannel();
      }
    } else chan[ch] = new OggChannel();
#else
    warning("Stream_mixer::set_channel(%u,%u) : can't open OggVorbis (support not compiled)",ch,pos);
    return(0);
#endif
  } else if(strncasecmp(sel+strlen(sel)-4,".mp3",4)==0) {
    /* take care to delete the old channel if present*/
    if(chan[ch]!=NULL) {
      if(chan[ch]->type != MP3CHAN) {
	func("deleting previous OggChannel");
	delete (MpegChannel*)chan[ch];
	jsleep(0,100);
	chan[ch] = new MpegChannel();
      }
    } else chan[ch] = new MpegChannel();
  }

  unlock();

  /* ok, we have the channel here, let's load the bitstream */
  res = chan[ch]->set(sel);


  if(!res) {  /* there is an error in opening the file */
    error("Stream_mixer::set_channel can't set channel %u", ch);
    chan[ch]->opened = false;

    switch(chan[ch]->playmode) {
    case chan[ch]->LOOP: /* LOOP :::::::::::::::::::::::: */
      return(0);
      break;
    case chan[ch]->CONT: /* CONT :::::::::::::::::::::::: */
      if(playlist[ch].len()<=1) return(0);
      if(pos>=playlist[ch].len()) pos=1; else pos++;
      /* recursion HERE! et voila' */
      return set_channel(ch,pos);
      break;
    default: /* PLAY :::::::::::::::::::::::: */
      return(0);
      break;
    }

  } else { /* file opened, everything ok */

    /* flush out the pipe */
    chan[ch]->flush();

    if(!chan[ch]->running) { /* startup separated thread */
      chan[ch]->lock();
      chan[ch]->start();
      func("waiting for thread to start");
      chan[ch]->wait();
      /* wait locks when exits, so we unlock */
      chan[ch]->unlock();
    }

    func("SELECT ON PLAYLIST");
    playlist[ch].sel(pos);
    
    switch(chan[ch]->type) {
    case MP3CHAN:
      notice("Mpeg on chan[%u] %ukbit %ukhz %s"
	     , ch, chan[ch]->bitrate, chan[ch]->samplerate,
	     (chan[ch]->channels==1) ? "mono" : "stereo" );
      break;
    case OGGCHAN:
      notice("OggVorbis on chan[%u] %ukhz %s"
	     , ch, chan[ch]->samplerate,
	     (chan[ch]->channels==1) ? "mono" : "stereo" );
      break;
    }
    /* if have_gui select the choosen song */
    if(have_gui) gui->sel_playlist(ch,playlist[ch].selected_pos());

  }
  return(res);
}

void Stream_mixer::set_all_volumes(float *vol) {
  int ch;
  lock();
  for(ch=0;ch<MAX_CHANNELS;ch++) {
    if(chan[ch]!=NULL)
      chan[ch]->volume = vol[ch];
  }
  unlock();
}

void Stream_mixer::set_volume(int ch, float vol) {
  if(!chan[ch]) {
    warning("Stream_mixer::set_volume(%u,%f) called on a NULL channel",ch,vol);
    return;
  }

  lock();
  chan[ch]->volume = vol;
  unlock();
}

void Stream_mixer::crossfade(int ch1, float vol1, int ch2, float vol2) {
  if(!chan[ch1] || !chan[ch2]) {
    warning("Stream_mixer::crossfade(%u,%f,%u,%f) called on a NULL channel",ch1,vol1,ch2,vol2);
    return;
  }

  lock();
  chan[ch1]->volume = vol1;
  chan[ch2]->volume = vol2;
  unlock();
}

void Stream_mixer::set_speed(int ch, int speed) {
  lock();
  chan[ch]->speed = speed;
  unlock();
  /* poi lo processa l'inchannel dentro al metodo run()
     cioe' il resampling che fa li' quando lo prepara al mixing
  */
}

bool Stream_mixer::play_channel(int ch) {
  /* paranoia */
  if(chan[ch]==NULL) {
    warning("Stream_mixer::play_channel(%u) called on a NULL channel",ch);
    return(false);
  }

  func("Stream_mixer::play_channel(%i)",ch);
  
  lock();
  if(!chan[ch]->play())
    error("can't play channel %u",ch);
  unlock();
  return(chan[ch]->on);
}
  
bool Stream_mixer::stop_channel(int ch) {
  /* paranoia */
  if(!chan[ch]) {
    warning("Stream_mixer::stop_channel(%u) called on a NULL channel",ch);
    return(false);
  }

  bool res = false;
  if(chan[ch]->running) {
    lock();
    res = chan[ch]->stop();
    unlock();
    if(have_gui) gui->sel_playlist(ch,playlist[ch].selected_pos());
  }
  return(res);
}

bool Stream_mixer::set_position(int ch, float pos) {
  bool res = false;
  /* paranoia */
  if(!chan[ch]) {
    warning("Stream_mixer::set_position(%u,%f) called on a NULL channel",ch,pos);
    return(res);
  }

  if(!chan[ch]->opened) {
    error("can't seek position on channel %u",ch);
    return(res);
  }

  /*
  if(pos==1.0) {
    set_channel(ch,playlist[ch].sel()+1);
    return(res);
  }
  */

  if(chan[ch]->seekable && chan[ch]->running) {
    lock();
    chan[ch]->flush();
    chan[ch]->lock();
    res = chan[ch]->pos(pos);
    chan[ch]->unlock();
    if(!res)
    error("can't seek position %f on channel %u",pos,ch);
    /* here the set_position automatically plays the file
       this means if the file is paused or stopped and you move the position
       it starts playing.
       i'm not sure that's the best logic to use, but it's simple
       and it's fine to have it like that right now. // jaromil */
    chan[ch]->play();
    unlock();
  } else
    error("channel %u is not seekable",ch);
  return(res);
}

/* move song 'pos' in channel 'ch' to the new 'npos' in channel 'nch'
   songs can also be moved within the same channel */
bool Stream_mixer::move_song(int ch, int pos, int nch, int npos) {
  Entry *x = playlist[ch].pick(pos);
  if(x) {
    /* the insert also removes from the old list
       (in future we should be using the linklist API directly) */
    playlist[nch].insert(x,npos);
    return(true);
  }
  return(false);
}

bool Stream_mixer::set_live(bool stat) {
  if(dsp<1) {
    warning("ignoring live-in: soundcard not initialized");
    return(false);
  }
  
  if(!((dspout)&&(!fullduplex)&&(stat))) {
    lock();
    livein.on = linein = stat;
    unlock();
  }
  
  return(livein.on);
}

bool Stream_mixer::set_lineout(bool stat) {
  if(dsp<1) {
    error("soundcard is not initialized",stat);
    return(false);
  }

  if(!((livein.on)&&(!fullduplex)&&(stat))) {
    lock();
    dspout = stat;
    unlock();
  }
  return(dspout&stat);
}

void Stream_mixer::close_soundcard() {
  ioctl(dsp, SNDCTL_DSP_RESET, 0);
  close(dsp);
}

void Stream_mixer::set_playmode(int ch, int mode) {
  
  switch(mode) {
  case 0:
    chan[ch]->playmode = chan[ch]->PLAY;
    break;
  case 1:
    chan[ch]->playmode = chan[ch]->LOOP;
    break;
  case 2:
    chan[ch]->playmode = chan[ch]->CONT;
    break;
  }
}

/* this is the function selecting files for the scandir */
int selector(const struct dirent *dir) {
  if( strncasecmp(dir->d_name+strlen(dir->d_name)-4,".mp3",4)==0
#ifdef HAVE_VORBIS
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".ogg",4)==0
#endif
      || strncasecmp(dir->d_name+strlen(dir->d_name)-3,".pl",3)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".pls",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".m3u",4)==0 )
    return(1);

  struct stat prcd;
  stat(dir->d_name,&prcd);
  if(S_ISDIR(prcd.st_rdev)) return(1);
  
  return(0);
}
  
      
    

bool Stream_mixer::add_to_playlist(int ch, const char *file) {


  if(!file) {
    warning("Stream_mixer::add_to_playlist(%i,NULL) called",ch);
    return(false);
  }

  char temp[MAX_PATH_SIZE];
  /* in path i store the new allocated string into the playlist */
  char *path, *p;

  strncpy(temp,file,MAX_PATH_SIZE);
  chomp(temp);
  
  /* if it's a url, just add it */
  if(strncasecmp(file,"http://",7)==0) {
    lock();
    path = playlist[ch].addurl(temp);
    unlock();
    if(have_gui) gui->add_playlist(ch,path);
    return(true);
  }
  
  /* if it's a local file url (like gnome d&d)
     strip away the file:// and treat it normally */
  if(strncasecmp(file,"file://",7)==0) {
    strncpy(temp,&file[7],MAX_PATH_SIZE);
    func("QUAAA %s",temp);
  }
    
  
  /* if it's not a stream, check if the file exists and it's readable */
  FILE *fd = NULL;
  fd = fopen(temp,"r");
  if(!fd) {
    warning("Stream_mixer::add_to_playlist : %s is not readable",temp);
    return(false);
  } else fclose(fd);

  bool res = false;
  
  /* check if the file has a correct extension which is supported 
     and handle it if it's a playlist */

  /* IT's A MP3 OR OGG */
  if( strncasecmp(temp+strlen(temp)-4,".ogg",4)==0
   || strncasecmp(temp+strlen(temp)-4,".mp3",4)==0
      ) {
    lock();
    path = playlist[ch].addurl(temp);
    unlock();
    
    if(have_gui) {
      p = path+strlen(path);// *p='\0';
      while(*p!='/') p--; p++;
      gui->add_playlist(ch,p);
    }

    res = true;

    /* IT's A PLAYLIST */
  } else if( strncasecmp(temp+strlen(temp)-3,".pl",3)==0
	     || strncasecmp(temp+strlen(temp)-4,".pls",4)==0
	     || strncasecmp(temp+strlen(temp)-4,".m3u",4)==0 ) {
    /* the file is a playlist, read thru it and append it to the existing */
    char votantonio[MAX_PATH_SIZE];
    fd = fopen(temp,"r");
    while(fgets(votantonio,MAX_PATH_SIZE,fd)!=NULL) {
      chomp(votantonio);
      /* ET VOILA', RECURSION in one step out HERE (SENZA MANIII)
	 MARO', SO' NU MAGHE! ARRISUSCIT' LI MUORTE! MARONN'O CARMINE!
	 ECCHI ME FERME CCHIU'! AGGIA FATT' LA RICORSIOOONE! MAAROOOOOO!
	 .. ok, ho sclerato in modo male //jaromil
      */
      add_to_playlist(ch,votantonio);
    }
    fclose(fd);
    res = true;

    /* TRY IF IT's A DIRECTORY */
  } else {
    
    struct stat prcd;
    if(stat(temp,&prcd)<0) {
      error("can't read file status");
      warning("cannot stat %s : %s",temp,strerror(errno));

    } else if(prcd.st_mode & S_IFDIR) {
     
      func("Stream_mixer::add_to_playlist called on directory");
      struct dirent **filelist;
      int found = scandir(temp,&filelist,selector,alphasort);
      if(found<0) {
	error("file not found");
	warning("Stream_mixer::add_to_playlist(%u,%s) : %s",ch,file,strerror(errno));
      } else {
	int c;
	for(c=0;c<found;c++) {
	  char barakus[MAX_PATH_SIZE];
	  snprintf(barakus,MAX_PATH_SIZE,"%s/%s",temp,filelist[c]->d_name);
	  add_to_playlist(ch,barakus);
	}
	res = true;
      }
      
    } else {
      error("file extension is not recognized");
      warning("error adding %s (extension not recognized)",temp);
    }
  }

  return(res);
}

void Stream_mixer::rem_from_playlist(int ch, int pos) {
  /* paranoia */
  if(ch>MAX_CHANNELS) {
    warning("Stream_mixer::rem_from_playlist(%u,%u) : channel does'nt exists");
    return;
  }

  lock();

  playlist[ch].rem(pos);

  pos = (pos>playlist[ch].len()) ? playlist[ch].len() : pos;
  if(pos>0) {
    playlist[ch].sel(pos);
    if(have_gui) gui->sel_playlist(ch,pos-1);  
  }
  unlock();
}

int Stream_mixer::create_enc(enum codec enc) {
  OutChannel *outch = NULL;
  switch(enc) {

#ifdef HAVE_VORBIS
  case OGG:
    outch = new OutVorbis;

    if( ! ((OutVorbis*)outch)->init() ) {
      error("error initializing %s",outch->name);
      delete (OutVorbis*)outch;
      return -1;
    }
    break;
#endif

#ifdef HAVE_LAME
  case MP3:
    outch= new OutLame;
    if( ! ((OutLame*)outch)->init() ) {
      error("error initializing %s",outch->name);
      delete (OutLame*)outch;
      return -1;

    }
    break;
#endif

  default: break; /* placeholder */

  }
  
  outchans.add(outch);
  
  idseed += 1000; /* here is a limit of 1000 shouter ID slots for each encoder
		     i bet you'll not reach it */
  outch->id = idseed;
  
  notice("%s encoder succesfully initialized",outch->name);
  return outch->id;
}

void Stream_mixer::delete_enc(int id) {
  OutChannel *outch = (OutChannel*) outchans.pick_id(id);
  if(!outch) {
    warning("delete_enc: invalid encoder requested ID:%i",id);
    return;
  }

  lock();
  
  outch->rem();

  if(outch->running) {
    outch->quit = true;
    jsleep(0,50);
    outch->lock(); outch->unlock();
    outch->flush(); /* QUAA: CHECK THIS */
  }

  /*  
  switch(outch->tipo) {

#ifdef HAVE_VORBIS
  OGG: delete (OutVorbis*)outch; break;
#endif
  
#ifdef HAVE_LAME
  MP3: delete (OutLame*)outch; break;
#endif
  
  default: break;
  }
  */
  delete outch;
  unlock();
}

OutChannel *Stream_mixer::get_enc(int id) {
  return (OutChannel*)outchans.pick_id(id);
}

bool Stream_mixer::apply_enc(int id) {
  OutChannel *outch = (OutChannel*)outchans.pick_id(id);
  if(!outch) {
    warning("apply_enc: invalid encoder requested ID:%i",id);
    return false;
  }

  if(!outch->profile_changed) return true;

  char *qstr = outch->guess_bps();

  lock();
  outch->lock();

  outch->initialized = outch->apply_profile();

  outch->unlock();
  unlock();

  if(outch->initialized)
    notice("%s SET Q%u %s%s", outch->name, (int)fabs(outch->quality()),
	qstr, (outch->channels()==1)?" mono ":" stereo ");
  else
    error("ERROR %s SET Q%u %s%s",outch->name, (int)fabs(outch->quality()),
	  qstr, (outch->channels()==1)?" mono ":" stereo ");  

  return outch->initialized;
}

/* updchan takes care that any action is taken (channel ends etc.)
   and updates the gui if registered.
   this is called only by cafudda while the mixer is locked, so no need to lock
*/
void Stream_mixer::updchan(int ch) {
  if(chan[ch]->state>1.0) {
    if(chan[ch]->state==3.0)
      error("JMIX::updchan : channel %u reported errors",ch);
    
    chan[ch]->stop();

    switch(chan[ch]->playmode) {
    case chan[ch]->PLAY: /* PLAY :::::::::::::::::::::::: */
      unlock();
      if(!have_gui) quit = true;
      if(chan[ch]->seekable) set_channel(ch,playlist[ch].selected_pos());
      else if(have_gui) gui->sel_playlist(ch,playlist[ch].selected_pos());
      lock();
      break;
    case chan[ch]->LOOP: /* LOOP :::::::::::::::::::::::: */     
      unlock();
      if(chan[ch]->seekable) {
	set_channel(ch,playlist[ch].selected_pos());
	play_channel(ch);
      } else if(have_gui) gui->sel_playlist(ch,playlist[ch].selected_pos());
      lock();
      break;
    case chan[ch]->CONT: /* CONT :::::::::::::::::::::::: */
      int now = playlist[ch].selected_pos();
      int next =
	(now < playlist[ch].len()) ? now+1 : 1;

      unlock();
      set_channel(ch,next);
      play_channel(ch);
      lock();

      playlist[ch].sel(next);
      if(have_gui) gui->sel_playlist(ch,playlist[ch].selected_pos());

      break;
    }

  } else { /* state<1.0 : normal flow */
    if(have_gui && chan[ch]->seekable) {
      char temp[8];
      snprintf(temp,8,"%02u:%02u",chan[ch]->time.m,chan[ch]->time.s);
      if(strncmp(temp,gui->ch_lcd[ch],5)!=0) { /* LCD changed */
	strncpy(gui->ch_lcd[ch],temp,5);
	gui->set_lcd(ch, gui->ch_lcd[ch]);
      }
      if(gui->ch_pos[ch] != chan[ch]->state) { /* POSITION changed */
	gui->ch_pos[ch] = chan[ch]->state;
	gui->set_pos(ch, chan[ch]->state);
      }
    }
  }
}

/* this routine clips audio and calculates volume peak
   this saves cpu cycles by doing it all in the same iteration   
   featuring an adaptive coefficient for volume and clipping
   
   Copyright (C) 2002 Matteo Nastasi aka mop <nastasi@alternativeoutput.it>
*/


void Stream_mixer::clip_audio(int samples) {
  int c;
  static float k = 1.0;
  int pproc,sum = 0;
#ifdef MOP_LOGGING
  static int mopct = 0, supsum = 0;
#endif

  if(samples==0) return;
  int words = samples<<1;

  for(c=0;c<words;c++) {
    /* value of the attenuated sample */
    pproc = (int)(((float)(process_buffer[c]))*k);

    if(pproc > 32767) {
      /* sum of the exceeding area for tne computation of the current k val */
      sum += (pproc-32767);
      audio_buffer[c] = peak[cpeak] = 32767;
    }
    else if(pproc < -32768) {
      /* sum of the exceeding area for ... */
      sum += (-pproc-32768);
      audio_buffer[c] = -32768;
      }
    else {
      audio_buffer[c] = (short)pproc;
      if (pproc>peak[cpeak])
	peak[cpeak] = pproc;
    }
  }
  k = (k * MOP_ADV_RETM + 
       1.0 / ((1.0 + MOP_ADV_KARE * (sum / (float)(samples*32767))))) / (MOP_ADV_RETM + 1.0);
  
#ifdef MOP_LOGGING        
  /* every 128 chunks print the current k value and the average of exceeding area */     
  if ((mopct % 128) == 0) {
    supsum >>= 7;
    warning("JMIX::clip_audio(%i) : k = (%f,%ld)",samples,k,supsum);
    supsum = sum;
  }
  else
    supsum += sum;
  mopct++;
#endif

}
