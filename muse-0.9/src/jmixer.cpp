/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2004 Denis Roio aka jaromil <jaromil@dyne.org>
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

#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/time.h>

#include <config.h>

#include <jutils.h>
#include <audioproc.h>
#include <jmixer.h>
#include <playlist.h>
#include <inchannels.h>
#include <dev_sound.h>

#ifdef HAVE_VORBIS
#include <out_vorbis.h>
#endif

#ifdef HAVE_LAME
#include <out_lame.h>
#endif

#define CODENAME "STREAMTIME"



/* process_buffer BUF_SIZE is:
   BUF_SIZE of 32bit ints *2channels *8resampling_space

   audio_buffer BUF_SIZE is:
   BUF_SIZE of 16bit short *2channels *8resampling_space
*/

#define PARACHAN \
  if(!chan[ch]) { \
    warning("%i:%s %s - channel %i is NULL", \
    __LINE__,__FILE__,__FUNCTION__,ch); \
    return(false); \
  }


Stream_mixer::Stream_mixer() {
  int i;
  for(i=0;i<MAX_CHANNELS;i++)
    chan[i] = NULL;
#ifdef HAVE_SCHEDULER
  register_sched(NULL); 
#endif

  /* here memset takes byte num */
  memset(process_buffer,0,PROCBUF_SIZE*sizeof(int32_t));
  memset(audio_buffer,0,PROCBUF_SIZE*sizeof(int16_t));

  dsp = 0;
  max = 0;
  have_gui = false;

  dspout = false;
  linein = false;
  linein_vol = 1;
  fileout = false;
  quit = false;

  for(i=0;i<8;i++) peak[i] = 0;
  cpeak = 0;

  // create the Sound Device controller class
  snddev = new SoundDevice();
  if( snddev->open(true,true) ) {
    dsp = 1;
    fullduplex = true;
  }

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
    snddev->close();
    delete snddev;
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

bool Stream_mixer::open_soundcard(bool in, bool out) {
  if( ! snddev->open(in,out) ) return false;
  dsp = 1;
  fullduplex = true;
  return true;
}

void Stream_mixer::close_soundcard() {
  snddev->close();
}

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

      /*
      if(chan[i]->update) {
	if(have_gui) 
	  gui->sel_playlist
	    (i,chan[i]->playlist->selected_pos());
	chan[i]->update = false;
      }
      */

      if(chan[i]->on) {	

	// this read from pipe is set to mix int32 down to the process_buffer
	cc = chan[i]->erbapipa->read(MIX_CHUNK<<1,process_buffer);


	// if(cc!=MIX_CHUNK<<2) warning("hey! mix16stereo on ch[%u] returned %i",i,cc);
	if(cc<0) continue;
	//	c+=cc<<1;
	c+=cc;

	if(have_gui)
	  if(chan[i]->update) {
	    updchan(i);
	    chan[i]->update = false;
	  }
      } /* if(chan[i].on) */

    } /* if(chan[i] != NULL) */
  } /* for(i=0;i<MAX_CHANNELS;i++) */


  if(linein) {
#ifndef PORTAUDIO
    // ires = livein.mix(process_buffer);
    c += livein.mix(process_buffer);
    // max = (max<ires) ? ires : max;
#else
    linein_samples = snddev->read(linein_buf,MIX_CHUNK);
    for(cc=0; cc<linein_samples<<1; cc++) { //<<1 stereo
      
      // mix and multiply for the volume coefficient
      process_buffer[cc] += (int32_t) (linein_buf[cc] * linein_vol);
      
    }
    c += linein_samples;
#endif
  }



  
#ifdef HAVE_SCHEDULER
  if (rsched && rsched->channel->opened) {
    c += rsched->channel->erbapipa->read(MIX_CHUNK,process_buffer);
  }
#endif

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

      if(out->encoding
	 && out->initialized
	 && out->running) {

	out->erbapipa->write(MIX_CHUNK<<2,audio_buffer);
	total_bitrate += out->get_bitrate();

      }

      out = (OutChannel*) out->next;

    }

    /* WRITE 2 DSP */
    if(dspout) {
      /* write out interleaved stereo 16bit pcm 
	 dsp takes number of *BYTES*, the format
	 is being setted with ioctls in initialization */
#ifndef PORTAUDIO
      write(dsp,audio_buffer,MIX_CHUNK<<2);
#else
      snddev->write(audio_buffer,MIX_CHUNK<<1); // always stereo
#endif

      //      do {ret=write(dsp,audio_buffer,MIX_CHUNK<<2);} while (ret==-1 && errno==EINTR);
      // what was that? there shouldn't be a loop on the audiocard write -jrml
    }
    
    /* compute and draw levels */
    cpeak++;
    if(have_gui 
       && cpeak==8 
       && gui->meter_shown()) {
      // integer only weighted media over 8 elements -jrml
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
    } else // no GUI and no channels playing: quit CLI
      quit = true;

  }

  /* notice the gui to refresh */
  if(have_gui) gui->signal();
  
  /* we don't want massive usage of the cpu
     also thread synchronization is a shamanic practice ;)
     in which we must find the right moment to breath;
     
     here we give fifos a bit of air and avoid tight loops
     making the mixing engine wait 20 nanosecs */
  jsleep(0,20);

}

bool Stream_mixer::create_channel(int ch) {
  
  /* paranoia */
  if(chan[ch]) {
    warning("channel %i allready exists");
    unlock();
    return true;
  }

  Channel *nch;
  nch = new Channel();

  nch->lock();
  nch->start();
  func("waiting for channel %i thread to start",ch);
  nch->wait();
  /* wait for the existance lock, then we unlock */
  nch->unlock();

  lock();
  chan[ch] = nch;
  unlock();
  
  return(true);
}

bool Stream_mixer::delete_channel(int ch) { 
  /* paranoia */
  PARACHAN

  lock();
  /*
  if(chan[ch]->on) chan[ch]->stop();
  // quit the thread
  if(chan[ch]->running) {
    chan[ch]->quit = true;
    // be sure it quitted
    chan[ch]->signal();
    jsleep(0,50);
    chan[ch]->lock(); chan[ch]->unlock();

  }
  */

  /* clean internal allocated buffers */
  delete chan[ch];
  chan[ch] = NULL;
  //  chan[ch]->playlist->cleanup();
  unlock();
  return true;
}

bool Stream_mixer::pause_channel(int ch) {
  /* paranoia */
  PARACHAN

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
      return true;
    }
  } else warning("tried to switch pause on unopened channel %i",ch);
  return false;
} /* overloaded non-switching function follows */
bool Stream_mixer::pause_channel(int ch, bool stat) { /* if stat==true -> pause the channel */
  /* paranoia */
  PARACHAN

  if(chan[ch]->opened) {
    if(!stat) {
      lock();
      if(!chan[ch]->play())
	error("can't play channel %u",ch,ch);
      unlock();
    } else {
      chan[ch]->on = false;
      return true;
    }
  } else error("can't pause unopened channel %i",ch);
  return false;
}

bool Stream_mixer::set_channel(int ch, int pos) {
  PARACHAN

    if(!chan[ch]->playlist->sel(pos))
      return(false);
    else
      chan[ch]->opened = false;

  /* if have_gui select the choosen song
  if(have_gui)
    gui->sel_playlist( ch , pos );
  */
  return(true);
}

/*
  play the selected stream sound on the channel
  the file/stream is loaded
  (CHANGES TO API! RUBIK PERDONO)
  takes only the channel number
  ** int pos starts from 1
  set_channel returns:
  0 - error
  1 - bitstream opened (seekable)
  2 - bitstream opened (non seekable)
*/
int Stream_mixer::play_channel(int ch) {
  int res = 0;

  /* paranoia */
  PARACHAN

  lock();
  if(!chan[ch]->play())
    error("can't play channel %u",ch);
  else
    res = (chan[ch]->seekable) ? 1 : 2;
  unlock();

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

bool Stream_mixer::set_volume(int ch, float vol) {
  /* paranoia */
  PARACHAN

  lock();
  chan[ch]->volume = vol;
  unlock();
  return true;
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

bool Stream_mixer::stop_channel(int ch) {
  /* paranoia */
  PARACHAN

  bool res = false;
    //  if(chan[ch]->running) {
  lock();
  res = chan[ch]->stop();
  unlock();
  /*  if(have_gui) {
    int p = chan[ch]->playlist->selected_pos();
    if(p) gui->sel_playlist(ch,p);
    } */
  return(res);
}

bool Stream_mixer::set_position(int ch, float pos) {

  /* paranoia */
  PARACHAN
  bool res = false;
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
    //    chan[ch]->erbapipa->flush();
    chan[ch]->lock();
    res = chan[ch]->pos(pos);
    chan[ch]->unlock();
    if(!res) error("can't seek position %f on channel %u",pos,ch);
    // chan[ch]->play(); - this shouldn't be needed
    unlock();
  } else
    error("channel %u is not seekable",ch);
  return(res);
}

/* move song 'pos' in channel 'ch' to the new 'npos' in channel 'nch'
   songs can also be moved within the same channel */
bool Stream_mixer::move_song(int ch, int pos, int nch, int npos) {
  Entry *x = chan[ch]->playlist->pick(pos);
  func("move song %i on channel %i to channel %i in position %i",
       pos,ch,nch,npos);
  if(x) {
    /* the insert also removes from the old list
       (in future we should be using the linklist API directly) */
    chan[nch]->playlist->insert(x,npos);
    return(true);
  } else 
    func("no song to move there!");

  return(false);
}

bool Stream_mixer::set_live(bool stat) {
#ifndef PORTAUDIO
  if(dsp<1) {
    warning("ignoring live-in: soundcard not found");
    return(false);
  }
  
  if(!( (dspout)
	&&(!fullduplex)
	&&(stat)) ) {
    lock();
    livein.on = linein = stat;
    unlock();
  }
  
  return(livein.on);
#else
  lock();
  if( snddev->input(stat) )
    linein = stat;
  unlock();
  return linein;
#endif
}

void Stream_mixer::set_mic_volume(int vol) {
  lock();
  linein_vol = vol;
  unlock();
}
  
  

bool Stream_mixer::set_lineout(bool stat) {
#ifndef PORTAUDIO
  if(dsp<1) {
    error("ignoring sound output: soundcard not found");
    return(false);
  }

  if(!( (livein.on)
	&&(!fullduplex)
	&&(stat)) ) {
    lock();
    dspout = stat;
    unlock();
  }
  return(dspout&stat);
#else
  lock();
  if( snddev->output(stat) )
    dspout = stat;
  unlock();
  return dspout;
#endif
}

bool Stream_mixer::set_playmode(int ch, int mode) {
  
  switch(mode) {
  case PLAYMODE_PLAY:
    chan[ch]->playmode = PLAYMODE_PLAY;
    break;
  case PLAYMODE_LOOP:
    chan[ch]->playmode = PLAYMODE_LOOP;
    break;
  case PLAYMODE_CONT:
    chan[ch]->playmode = PLAYMODE_CONT;
    break;
  }
  return true;
}

/* this is the function selecting files for the scandir
   on freebsd systems you should change the following line to:
   int selector(struct dirent *dir) {    */
int selector(const struct dirent *dir) {
  if( strncasecmp(dir->d_name+strlen(dir->d_name)-4,".mp3",4)==0
#ifdef HAVE_VORBIS
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".ogg",4)==0
#endif
#ifdef HAVE_SNDFILE
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".wav",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".aif",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-5,".aiff",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".snd",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-3,".au",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".raw",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".paf",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".iff",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".svx",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-3,".sf",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".voc",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".w64",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".pvf",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-3,".xi",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".htk",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".mat",4)==0
#endif
      || strncasecmp(dir->d_name+strlen(dir->d_name)-3,".pl",3)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".pls",4)==0
      || strncasecmp(dir->d_name+strlen(dir->d_name)-4,".m3u",4)==0 )
    return(1);

//  struct stat prcd;
//  stat(dir->d_name,&prcd);
//  if(S_ISDIR(prcd.st_rdev)) return(1);
  
  return(0);
}
  
      
    

bool Stream_mixer::add_to_playlist(int ch, const char *file) {

  if(!file) {
    warning("Stream_mixer::add_to_playlist(%i,NULL) called",ch);
    return(false);
  }

  if(!chan[ch]) {
    warning("%i:%s %s - called on NULL channel %i",
	    __LINE__,__FILE__,__FUNCTION__,ch);
    warning("call jmixer::create_channel first");
    return(false);
  }

  char temp[MAX_PATH_SIZE];
  /* in path i store the new allocated string into the playlist */
  char *path, *p;

  strncpy(temp,file,MAX_PATH_SIZE);
  chomp(temp);
  func("add to playlist %s", temp);
  /* if it's a url, just add it */
  if(strncasecmp(temp,"http://",7)==0) {
    func("it's a network stream url");
    //    lock();
    path = chan[ch]->playlist->addurl(temp);
    //    unlock();
    if(have_gui) gui->add_playlist(ch,path);
    return(true);
  }
  
  /* if it's a local file url (like gnome d&d)
     strip away the file:// and treat it normally */
  if(strncasecmp(temp,"file://",7)==0) {
    func("it's a file url (drag & drop)");
    strncpy(temp,&file[7],MAX_PATH_SIZE);
    path = chan[ch]->playlist->addurl(temp);
    if(have_gui) gui->add_playlist(ch,path);
    return(true);
  }

  if(strncasecmp(temp,"jack://",7)==0) { // it's a JACK CLIENT
#ifdef HAVE_JACK
    func("it's a jack client input channel");
    strncpy(temp,file,MAX_PATH_SIZE);
    path = chan[ch]->playlist->addurl(temp);
    if(have_gui) gui->add_playlist(ch,path);
    return(true);
#else
    error("jack daemon support not compiled, client \'%s\' cannot be activated",&temp[7]);
    return(false);
#endif
  }
  
  /* if it's not a stream, check if the file exists and it's readable */
  FILE *fd = NULL;
  fd = fopen(temp,"r");
  if(!fd) {
    warning("is not a readable file",temp);
    return(false);
  } else fclose(fd);

  bool res = false;
  
  /* check if the file has a correct extension which is supported 
     and handle it if it's a playlist */

  /* IT's A MP3 OR OGG OR WAV */
  if( strncasecmp(temp+strlen(temp)-4,".ogg",4)==0
      || strncasecmp(temp+strlen(temp)-4,".mp3",4)==0
      || strncasecmp(temp+strlen(temp)-4,".wav",4)==0
      || strncasecmp(temp+strlen(temp)-4,".aif",4)==0
      || strncasecmp(temp+strlen(temp)-5,".aiff",4)==0
      || strncasecmp(temp+strlen(temp)-4,".snd",4)==0
      || strncasecmp(temp+strlen(temp)-3,".au",4)==0
      || strncasecmp(temp+strlen(temp)-4,".raw",4)==0
      || strncasecmp(temp+strlen(temp)-4,".paf",4)==0
      || strncasecmp(temp+strlen(temp)-4,".iff",4)==0
      || strncasecmp(temp+strlen(temp)-4,".svx",4)==0
      || strncasecmp(temp+strlen(temp)-3,".sf",4)==0
      || strncasecmp(temp+strlen(temp)-4,".voc",4)==0
      || strncasecmp(temp+strlen(temp)-4,".w64",4)==0
      || strncasecmp(temp+strlen(temp)-4,".pvf",4)==0
      || strncasecmp(temp+strlen(temp)-3,".xi",4)==0
      || strncasecmp(temp+strlen(temp)-4,".htk",4)==0
      || strncasecmp(temp+strlen(temp)-4,".mat",4)==0
      ) {
    func("it's a local file",temp);
    //    lock();
    path = chan[ch]->playlist->addurl(temp);
    //    unlock();
    
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
    func("it's a playlist");
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
      func("it's a directory");
      struct dirent **filelist;
      // this scandir had a problem browsing directories, now?
      int found = scandir(temp,&filelist,selector,alphasort);
      if(found<1) error("%i files found: %s",found,strerror(errno));
      else {
	int c;
	for(c=0;c<found;c++) {
	  char barakus[MAX_PATH_SIZE];
	  snprintf(barakus,MAX_PATH_SIZE,"%s/%s",temp,filelist[c]->d_name);
	  /* et vuala' la ricorsione pure qua */
	  add_to_playlist(ch,barakus);
	}
	res = true;
      }
      
    } else {
      error("file extension is not recognized");
      error("can't add to playlist %s",temp);
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

  //  lock();

  chan[ch]->playlist->rem(pos);

  pos = (pos>chan[ch]->playlist->len()) ?
    chan[ch]->playlist->len() : pos;
  if(pos>0) {
    chan[ch]->playlist->sel(pos);
    if(have_gui) gui->sel_playlist(ch,pos-1);  
  }
  //  unlock();
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
  
  notice("%s %s initialized",outch->name,outch->version);
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
    outch->flush(); /* QUA we waste some buffer in the pipe 
		       CHECK THIS */
  }

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

  //  if(!outch->profile_changed) return true;

  char *qstr = outch->quality_desc;

  lock();
  outch->lock();

  outch->initialized = outch->apply_profile();

  outch->unlock();
  unlock();

  if(outch->initialized)
    notice("%s quality %uKbps/s %uHz %s", outch->name, outch->bps(), outch->freq(),
	   (outch->channels()==1)?" mono ":" stereo ");
  else
    error("ERROR setting %s to quality %uKbps/s %uHz %s", outch->name, outch->bps(), outch->freq(),
	   (outch->channels()==1)?" mono ":" stereo ");
  
  return outch->initialized;
}



void Stream_mixer::updchan(int ch) {
  if(!chan[ch]) return;
  if(chan[ch]->seekable) {
    snprintf(gui->ch_lcd[ch],9,"%02u:%02u:%02u",
	     chan[ch]->time.h,chan[ch]->time.m,chan[ch]->time.s);
    //	if(strncmp(temp,gui->ch_lcd[ch],5)!=0) { // LCD changed */
    //strncpy(gui->ch_lcd[ch],temp,5);
    gui->set_lcd(ch, gui->ch_lcd[ch]);
    //	func("%i: %s %f",ch,gui->ch_lcd[ch],chan[ch]->state);
    //	}
    //	if(gui->ch_pos[ch] != chan[ch]->state) { /* POSITION changed */
    gui->ch_pos[ch] = chan[ch]->state;
    gui->set_pos(ch, chan[ch]->state);
    //	}
  }
}

/** this routine clips audio and calculates volume peak
   this saves cpu cycles by doing it all in the same iteration   
   featuring an adaptive coefficient for volume and clipping 
   Copyleft (C) 2002 Matteo Nastasi aka mop <nastasi@alternativeoutput.it> */
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
       1.0 / ((1.0 + MOP_ADV_KARE *
	       (sum / (float)(samples*32767))
	       ))
       ) / (MOP_ADV_RETM + 1.0);
  
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
