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




 THERE IS SOME DOCUMENTATION in this file, just look thru the code and
 if you are familiar with C/C++ then you'll maybe find useful stuff for
 you too :)




 "$Id$"

 */

#ifndef __JMIXER_H
#define __JMIXER_H

#include <math.h>
#include <pthread.h>

#include <inchannels.h>
#include <outchannels.h>
#include <gui.h>
#include <playlist.h>

#include <generic.h>

class Stream_mixer {

 private:
  
  GUI *gui;
  bool have_gui;
  
  int dsp;
  int max; /* lenght of actual buffer in bytes */
  int peak[8]; /* volume peak array*/
  int cpeak;
  bool fullduplex;

  int idseed;

  void updchan(int ch);
  void clip_audio(int samples);
  
  OutChannel *out; /* pointer for internal use */

  pthread_mutex_t _mutex;
  pthread_cond_t _cond;

 public:
  Stream_mixer();
  ~Stream_mixer();

  /* pthread stuff */
  void lock() { pthread_mutex_lock(&_mutex); };
  void unlock() { pthread_mutex_unlock(&_mutex); };
  void wait() { pthread_cond_wait(&_cond,&_mutex); };
  void signal() { pthread_cond_signal(&_cond); };
  /* ------------- */



  /* ===== GUI API - the interesting interface to use JMIXER */




  /* ======= INPUT PLAYER CHANNELS */

  bool create_channel(int ch);

  bool add_to_playlist(int ch, const char *file); /* this adds a playlist to the channel
						     can be many kinds of files:
						     1) .mp3 single files
						     2) .ogg single files
						     3) .pls | .pl | .m3u playlists
						     4) http:// mp3 streams
						     5) directory/ 
						        (recusively adding recognized contents) */
  void rem_from_playlist(int ch, int pos);

  int set_channel(int ch, int playlist_pos);

  void delete_channel(int ch);

  bool play_channel(int ch);

  bool stop_channel(int ch);

  void pause_channel(int ch);

  void pause_channel(int ch, bool stat);

  void set_all_volumes(float *vol);

  void set_volume(int ch, float vol);

  void crossfade(int ch1, float vol1, int ch2, float vol2);

  void set_playmode(int ch, int mode); /* this can take a value from 0 to 2:
					  LOOP, CONT, PLAY */

  /* VERY EXPERIMENTAL */ void set_speed(int ch, int speed);

  bool set_position(int ch, float pos);

  bool move_song(int ch, int pos, int nch, int npos);
  
  bool set_live(bool stat); /* this is the live input from soudcard */
  bool set_lineout(bool stat); /* this is the soundcard output */

  /* ======= OUTPUT ENCODER CHANNELS */
 
  /* create a encoder channel
     returns the ID of the encoder (keep it!)
     returns -1 on ERROR */
  int create_enc(enum codec enc); /* enc value can be MP3 or OGG
				is a enum defined in outchannels.h */
  void delete_enc(int id);
  OutChannel *get_enc(int id); /* returns the encoder object
				  used to configure its settings */
  bool apply_enc(int id); /* apply the current configuration */
  
  /* the following is done in muse.cpp main function
     the interface does'nt needs to deal with them */
  void register_gui(GUI *reg_gui);  /* tells the mixer there is a GUI */
  bool open_soundcard();
  void close_soundcard();

  /* THIS IS THE MAIN MIXING PROCEDURE
     CAFUDDA! */
  void cafudda();


  /* notice if we have a soundcard */
  bool dsp_ok() { if(dsp>0) return true; else return false; };


  /* channels and playlists */
  Channel *chan[MAX_CHANNELS];
  Playlist playlist[MAX_CHANNELS];

  /* live soundcard input */
  LiveIn livein;

  /* encoder outchannels */
  Linklist outchans;
  OutChannel *lame;
  OutChannel *ogg;
  
  /* commandline vumeter */
  bool cli_vumeter;
  void cli_vumeter_set(int val);

  bool dspout;
  bool linein;
  bool fileout;
  bool quit;
  
  /* mixing buffers */
  int16_t audio_buffer[PROCBUF_SIZE];
  int32_t process_buffer[PROCBUF_SIZE];

};

#endif
