/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2002 Denis Rojo aka jaromil <jaromil@dyne.org>
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
 */

/**
   @file outchannels.h MuSE Output Channels interface
   
   @desc This is the external API offered by OutChannel instances.
   They are returned by create_enc method found in Stream_mixer.
*/

#ifndef __OUTCHANNELS_H__
#define __OUTCHANNELS_H__

#include <pthread.h>
#include <shouter.h>
#include <pipe.h>
#include <linklist.h>
#include <generic.h>
#include <resample/samplerate.h>

#define ENCBUFFER_SIZE 128000 // 65536 // we have ram, isn't it?



/**
   @enum OutChannel codec type
   @brief an enum holding the type of encoders available */
enum codec {
  MP3, ///< Lame encoder 
  OGG  ///< Ogg/Vorbis encoder
};

/**

   Instances of the OutChannel class can be crieated via the method
   create_enc method found in the Stream_mixer class.

   @class OutChannel   
   @brief the Output Channel class
*/
class OutChannel: public Entry {

  /**
     @defgroup outchannel OutChannel encoder

     The following methods and properties are publicly available
     to control the behaviour or a OutChannel instance.

     OutChannel offers you:

     uniformed settings for the parameters of each codec:

     - bps ( Kbit/s )
     - freq ( samplerate in Hz )
     - channels (1 is mono, 2 is stereo)
     - quality (VALUE from 0.1 to 9.0)
     - lowpass (lowpass in Hz, only with MP3)
     - highpass (highpass in Hz, only with MP3)

     methods to create instances of the Shouter class (*_ice)
     
     methods to start/stop the file dumping of the encoded audio to a
     certain file.

     The codec parameters are declared thru defines like INT_SET or
     CHAR_SET or FLOAT_SET: they basically define two overloaded
     functions to set and get the named parameter.

     @{
  */


 public:
  /**
     @brief the OutChannel class constructor 
     @param myname given at creation by Stream_mixer
  */
  OutChannel(char *myname);
 
  virtual ~OutChannel(); ///< the OutChannel class destructor

  
  char name[128]; ///< name string of the encoder type (read only)
  char version[128]; ///< version string of the encoder type (read only)
  enum codec tipo; ///< codec type (read only)

  bool quit; ///< set to true to exit the OutChannel thread
  bool running; ///< if true the OutChannel thread is running (read only)
  bool initialized; ///< if true all buffers have been allocated (read only)


  //////////////////////////////////////
  // SHOUTERS
  //////////////////////////////////////
  
  /**
     @brief Create a Shouter instance
     @returns ID of the new Shouter instance, or -1 on error
  */
  int create_ice();

  /**
     @brief delete a Shouter with ID
     @param iceid Shouter ID
  */
  bool delete_ice(int iceid);

  /**
     @brief get a Shouter instance with ID
     @param iceid Shouter ID
     @return il puntatore ad uno Shouter
  */
  Shouter *get_ice(int iceid);

  /**
     @brief Applica la configurazione corrente.

     Se gia' connesso, resta connesso
     
     @param iceid Shouter ID
     @return true on success, false otherwise
  */
  bool apply_ice(int iceid);

  /** 
      @brief Connette o disconnette il server ID a seconda del flag.
      
      @param iceid Shouter ID
      @param on on/off flag
      @return true if connected, false otherwise
  */
  bool connect_ice(int iceid, bool on);
  
  /** @brief the Linklist of Shouter instances
      
  This is directly accessible: position operations on the Linklist class
  are thread safe.
  */
  Linklist icelist;

  /////////////////////////////////////
  // end of Shouters
  /////////////////////////////////////



  /////////////////////////////////////
  // ENCODER SETTINGS HERE
  /////////////////////////////////////

  INT_SET(bps,_bps); ///< Kbit/s

  INT_SET(freq,_freq); ///< samplerate in Hz

  INT_SET(channels,_channels); ///< channels (1 is mono, 2 is stereo)

  INT_SET(lowpass,_lowpass); ///< lowpass in Hz

  INT_SET(highpass,_highpass); ///< highpass in Hz

  /**
     This method guesses the bps and samplerate parameters of the
     encoder from quality value, then it renders the quality_desc
     string with a human readable description of the setting.
     
     You can internally tweak this function to modify the mapping
     of quality values to bps and samplerate.

     @brief setup the bps encoder value
     @return pointer to quality desc  */     
  char *quality(float in); ///< setup quality (wraps most useful modes)
  float _quality;
  char quality_desc[256]; ///< string rendered to describe the quality of encoding

  /**
     @brief get the size of encoded audio in bytes
     @return the size of the encoded audio in bytes
  */
  unsigned int get_bitrate() { return bitrate; };



  ///////////////////////////////////////
  // DUMP TO FILE
  ///////////////////////////////////////

  /** 
      Starts to dump the encoded audio inside a local file.
      If the file is allready existing, it creates a new one
      with a slightly different name, without overwriting.
      If it was allready dumping, it keeps on: to change the
      filename to another file you must stop and then restart.
      
      @brief Start encoding to a file
      @param file full path to the encoded file, including extension
      @return true on success, false otherwise
  */
  bool dump_start(char *file);
  /**
     @brief Stop encoding to a file
     @return true on success, false otherwise
  */
  bool dump_stop();

  FILE *fd; ///< if non-zero a file is opened for dumping
  char fd_name[MAX_PATH_SIZE]; ///< full path filename for dumping

  
  // end of the OutChannel public interface
  /// @}


  ///////////////////////////////////
  // IF YOU ARE DEALING WITH A GUI
  //  YOU ARE NOT INTERESTED IN WHAT
  // COMES NEXT TO HERE
  ///////////////////////////////////


  /**     
	  Used to activate the settings, after every change.

	  To be able to change the OutChannel settings on the fly
	  while encoding thread is running you should never call this
	  directly, but only thru the Stream_mixer::apply_enc method.

	  This is a pure virtual function, the implementation is defined
	  inside the codec classes inheriting OutChannel.
	  
	  @brief apply changes to the parameters // INTERNAL USE, use Stream_mixer::apply_enc instead
	  @return true on success, false otherwise
  */
  virtual bool apply_profile() =0;

  
  virtual bool init() = 0; ///< pure virtual function (INTERNAL)
  virtual int encode() =0; ///< pure virtual function (INTERNAL)
  virtual void flush() =0; ///< pure virtual function (INVERNAL)


  /* === */
  int shout(); ///< strimma fuori l'encodato a tutti gli Shouter // INTERNAL USE by run()
  bool dump(); ///< scrive fuori l'encodato nel file // INTERNAL USE by run()

  /**
     @brief feeds the pipe for encoding // INTERNAL USE by jmixer.cpp carfudda()
     @param data audio buffer to be encoded
     @param len length in bytes of audio buffer to encode
     @return
     - int>0 = quantity of data pushed into the pipe
     - 0 = no need to feed in data: no encoding is configured
     - -1 = pipe locked, wait a bit
  */
  void push(void *data, int len);  
  Pipe *erbapipa; ///< Pipe instance to feed the encoder // INTERNAL USE by run()
  bool encoding; ///< flag checked by run, (streaming||fd) ? true : false // INTERNAL USE by run()

  /* pthread methods */
  void start(); ///< start OutChannel thread
  void run(); ///< running loop called by start()
  void lock() ///< lock OutChannel main thread mutex
    { pthread_mutex_lock(&_mutex); };
  void unlock() { pthread_mutex_unlock(&_mutex); }; ///< unlock OutChannel main thread mutex
  void wait() { pthread_cond_wait(&_cond,&_mutex); }; ///< wait for OutChannel thread signal
  void signal() { pthread_cond_signal(&_cond); }; ///< launch the OutChannel thread signal
  void lock_ice() { pthread_mutex_lock(&_mutex_ice); }; ///< lock Shouters thread
  void unlock_ice() { pthread_mutex_unlock(&_mutex_ice); }; ///< unlock Shouters thread
  void destroy() { _thread_destroy(); };
  ///< destroy the OutChannel thread // INTERNAL USE, use quit = true instead
  /* ------------- */  

  int16_t buffer[ENC_BUFFER]; ///< buffer holding the encoded audio

 private:


  /* pthread properties and private methods */
  void _thread_init(); ///< pthread_init
  void _thread_destroy(); ///< pthread_destroy
  bool _thread_initialized; ///< is_pthread_initialized
  
  pthread_t _thread; ///< thread pointer
  pthread_attr_t _attr; ///< thread attribute
  pthread_mutex_t _mutex; ///< OutChannel thread mutex
  pthread_mutex_t _mutex_ice; ///< Shouters thread mutex
  pthread_cond_t _cond; ///< OutChannel thread condition
  /* ------------- */

  int idseed; ///< unique ID pseudo-random seed

 protected:

  static void* kickoff(void *arg) { ((OutChannel *) arg)->run(); return NULL; };
  ///< kickoff the thread

  bool calc_bitrate(int enc); ///< bitrate calculus
  int encoded; ///< encoded buffer size in bytes
  int bitrate; ///< bits per second
  double now; ///< time now (for bitrate calculus)
  double prev; ///< time previous encoding (for bitrate calculus)
  unsigned int bytes_accu; ///< bytes accuracy

};



#endif
