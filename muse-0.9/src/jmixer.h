/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2004 Denis Rojo aka jaromil <jaromil@dyne.org>
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


/**
   @file jmixer.h MuSE mixer public interface header

   @desc this is the main external API to use MuSE
*/

#ifndef __JMIXER_H
#define __JMIXER_H

#include <math.h>
#include <pthread.h>

#include <inchannels.h>
#include <outchannels.h>
#include <gui.h>

#include <generic.h>

#ifdef HAVE_SCHEDULER
#include "radiosched.h"
#endif

class SoundDevice;

/**
   @class Stream_mixer

   @brief  the Multiple Streaming Engine class
     
Everything is connected to Stream_mixer:
the instance of this class controls all existent input and output
channels attached to it, plus it holds the file descriptors for the
soundcard and all the necessary private buffers of MuSE.

The Stream_mixer runs thru the cafudda() function which should be always
called from the main (parent) thread of your program.

Child threads are spawned by some of the public functions offered by
this class, anyway you don't have to take care of those issues.
*/
class Stream_mixer {

 public:
  Stream_mixer();
  ///< the Stream_mixer class constructor
  ~Stream_mixer();
  ///< the Stream_mixer class destructor

  ///////////////////////////////////////
  /// GUI API - the interesting interface 
  ///////////////////////////////////////




  /**
     @defgroup inchannels Stream_mixer input channels
     
     The following are methods implemented in the Stream_mixer class
     to control input channels (InChannel class) that are then mixed
     together and sent to output channels (OutChannel class).

     The input channels make audio chunks available thru buffered
     and threadsafe FIFO pipes (Pipe class), for the mixer to mix.
     
     The playlist functions are incorporated inside the
     Linklist and Playlist classes - which you can freely access.

     @{
  */
  
  /**
     At the moment MuSE gives only the possibility to have
     6 channels.
     
     This function creates the channel number ch and is necessary to
     call it before doing anything on the channel.
     
     @brief create channel ch
     @param ch number of the channel slot (from 1 to 6)
     @return true in case of success, false otherwise
  */
  bool create_channel(int ch);

  bool delete_channel(int ch); ///< deletes the channel ch
  
  /**
     This adds an input stream or playlist to a certain channel
     
     Many kinds of files are supported:
     -# .mp3 single files
     -# .ogg single files
     -# .pls | .pl | .m3u playlists
     -# http:// mp3 streams
     -# directory/ (recusively adding recognized contents)
     the types are recognized using the filename, more often
     the extension after the last dot or the prefix before the url.
     You can investigate the parser in jmixer.cpp.
     
     @param ch channel number
     @param file full path string
     @return true in case of success, false otherwise
  */
  bool add_to_playlist(int ch, const char *file);
  ///< inserts a new entry at the bottom of a channel playlist
  
  void rem_from_playlist(int ch, int pos);
  ///< removes the entry at a certain position from a channel playlist

  bool set_channel(int ch, int playlist_pos);
  ///< selects the entry at a certain position of a channel playlist

  /**
     Play the selected stream sound on the channel
     the file/stream is physically loaded here.
     takes only the channel number
     @param ch channel number starting from 1
     @return 0=error, 1=seekable 2=non-seekable
  */
  int play_channel(int ch);
  ///< set the channel playing

  bool stop_channel(int ch);
  ///< stop the channel

  bool pause_channel(int ch);
  ///< switch channel pause state between true and false
  
  bool pause_channel(int ch, bool stat);
  ///< set the channel pause state

  bool set_volume(int ch, float vol);
  ///< set a channel volume

  void set_all_volumes(float *vol);
  ///< set all channel volumes at once

  void crossfade(int ch1, float vol1, int ch2, float vol2);
  ///< set the volumes of two channels at once

  bool set_playmode(int ch, int mode);
  ///< set the playmode of a channel (LOOP,CONT,PLAY)

  void set_speed(int ch, int speed);
  ///< this is VERY EXPERIMENTAL, but we might get there soon
  
  bool set_position(int ch, float pos);
  ///< set the channel position (from 0.0 to 1.0)

  bool move_song(int ch, int pos, int nch, int npos);
  ///< move a playlist entry of a channel from a position to the other
  
  bool set_live(bool stat);
  ///< set the state of the live input from soundcard

  void set_mic_volume(int vol);
  ///< set the volume of the mic live input (sample multiplyer)

  bool set_lineout(bool stat);
  ///< set the state of the live output to soundcard
  
  /// @}
  


  /**
     @defgroup outchannels Stream_mixer output channels

     The following methods are implemented in the Stream_mixer class
     and can be used to control output channels (OutChannel class).

     
     Registered Outchannels are automatically feeded by the
     Stream_mixer thru buffered and threadsafe FIFO pipes (Pipe class)
     also every outchannel is running on his own thread.
     
     /// Internals:
     The superclass OutChannel wraps around two components:
     the Encoder, and the Shouter. The Encoder is internally
     implementing a Codec.

     @{ */
  
  /**
  
     The encoder channel is given back to the calling process in the
     form of a numeric (int) ID number.
     
     That number can be used as a reference to obtain back the
     Outchannel instance created.

     Internally this function starts up the Encoder thread and
     initialize its buffers for the Codec selected.

     @brief Create a new OutChannel encoder channel.
     @param enc can be MP3 or OGG (see enum codec in outchannels.h)
     @returns id the ID of the encoder, -1 on error
  */
  int create_enc(enum codec enc);

  /**
     @brief Delete an OutChannel encoder (discards ID).     
     @param id the ID of the OutChannel to destroy.
  */
  void delete_enc(int id);

  
  /**

     This function is used to obtain an OutChannel direct pointer to
     be able to configure it thru its public methods and properties.

     @brief Returns the OutChannel instance with the given ID.
     @param id the ID of the encoder
     @returns a pointer to the OutChannel class instance with ID
  */
  OutChannel *get_enc(int id);

  
  /**
    If any setting has been changed on the OutChannel, then this
    function must be called to apply them (works while running)
    
    @param id the ID of the encoder
    @returns true on success, false otherwise
  */
  bool apply_enc(int id);
  ///< apply the current OutChannel configuration
  
  /// @}




  
  /**
     Used to register a GUI instance to a Stream_mixer

     An example of using it is inside muse.cpp main()

     @param reg_gui GUI instance to register (see gui.h)
     @brief tell the mixer there is an GUI
  */
  void register_gui(GUI *reg_gui);


  SoundDevice *snddev;
  bool open_soundcard(bool in, bool out);
  void close_soundcard();

  ///< SoundDevice class for soundcard handling


  /**
     @brief THIS IS THE MAIN MIXING PROCEDURE: CAFUDDA!
     
     Cafudda should be repeatedly called by the main process.
     It doesn't fills up the CPU load so there is no need to wait
     or do any time syncronizing, it is just all done inside.

     Call it repeatedly after launching all the GUI threads and
     setting up the engine to act as you like, then while running
     on the main process the mixer can still be used by interface threads.
     
     see main.cpp for an example of its use.
  */
  void cafudda();


  /**
     @brief notice if we have a soundcard
     @return true if one was found
  */
  bool dsp_ok() { if(dsp>0) return true; else return false; };


  /** Array of Channels */
  Channel *chan[MAX_CHANNELS];

  /** live soundcard input */
  IN_DATATYPE linein_buf[PROCBUF_SIZE];
  int linein_samples;
  int linein_vol;

  LiveIn livein;

  /** encoder outchannels */
  Linklist outchans;
  
  /* the following are here filled with the first two
     encoder channels created. this is for backward
     compatibility with the existing GUIs, the encoders
     can be as many as you want! */
  OutChannel *lame;
  OutChannel *ogg;
  
 
  bool cli_vumeter; ///< commandline vumeter is experimental
  void cli_vumeter_set(int val); ///< so this is as well experimental

  bool dspout; ///< is there output to the soundcard speakers?
  bool linein; ///< is there some input from the microphone?
  bool fileout; ///< is there a file dumping? (redundant!)
  bool quit; ///< should we keep on running?





  //////////////////////////////////////////////
  /// MuSE mixer internal methods and properties
  //////////////////////////////////////////////
 private:
  
  GUI *gui;
  ///< pointer to the registered interface
  bool have_gui;
  ///< true if an interface is present
  int dsp; ///< soundcard file descriptor
  bool fullduplex; ///< true if soundcard is fullduplex  
  int max; ///< lenght of actual mix buffer in bytes */
  int peak[8]; ///< volume peak array
  int cpeak; ///< peak array counter
  int idseed; ///< pseudo-random seed for object IDs

  void updchan(int ch); ///< updates the gui display strings
  
  /** This routine clips audio and calculates volume peak
      this saves cpu cycles by doing it all in the same iteration   
      featuring an adaptive coefficient for volume and clipping,
      contributed by Matteo Nastasi aka mop (c) 2002
      
      @param number of samples (bytes*2*channels)
  */
  void clip_audio(int samples); ///< clip mixed audio
  
  OutChannel *out; ///< outchannel pointer for internal use


  int32_t process_buffer[PROCBUF_SIZE]; ///< mixing buffer
  int16_t audio_buffer[PROCBUF_SIZE]; ///< clipped mixing buffer


  /** Stream_mixer thread controls */
  void lock() { pthread_mutex_lock(&_mutex); };
  void unlock() { pthread_mutex_unlock(&_mutex); };
  void wait() { pthread_cond_wait(&_cond,&_mutex); };
  void signal() { pthread_cond_signal(&_cond); };
  pthread_mutex_t _mutex;
  pthread_cond_t _cond;

#ifdef HAVE_SCHEDULER
  Basic_scheduler *rsched;
  public:  void register_sched(Basic_scheduler *s) {rsched=s;};
#endif
};

#endif
