#ifndef __DEV_SOUND_H__
#define __DEV_SOUND_H__

#include <portaudio.h>
#include <pablio.h>

#include <config.h>
#ifdef HAVE_JACK
#include <jack/jack.h>
#endif
#include <pipe.h>

class SoundDevice {
 public:
  SoundDevice();
  ///< the SoundDevice class constructor
  ~SoundDevice();
  ///< the SoundDevice class destructor

  /**
     Tries to open the sound device for read and/or write
     if full-duplex is requested but not supported, it returns error
     and must be called again to fallback on half-duplex mode
     
     @param read true if device is opened for reading audio
     @param write true if device is opened for writing audio
     @return true in case of success, false otherwise
  */
  bool open(bool read, bool write);
  ///< open the sound device

  bool input(bool state); ///< activate sound input
  bool output(bool state); ///< activate sound output

  void close(); ///< close the sound device
  
  int read(void *buf, int len); ///< reads audio data from the device in a buffer, len is samples
  
  int write(void *buf, int len); ///< writes audio data from a buffer to the device, len is samples

  bool jack;
  bool jack_in;
  bool jack_out;


 private:
  PABLIO_Stream *aInStream; ///< Portaudio input stream 
  PABLIO_Stream *aOutStream; ///< Portaudio output stream 

  const PaDeviceInfo *info_input;
  const PaDeviceInfo *info_output;  

  bool pablio_input(bool state);
  bool pablio_output(bool state);
  
  PaError err;

  Pipe *jack_in_pipe;
  Pipe *jack_out_pipe;
#ifdef HAVE_JACK
  jack_client_t *jack_client;
  jack_port_t *jack_in_port;
  jack_port_t *jack_out_port;
  jack_default_audio_sample_t *jack_in_buf;
  jack_default_audio_sample_t *jack_out_buf;
  size_t jack_sample_size;
  int jack_samplerate;
#endif

};

#endif
