#ifndef __DEV_SOUND_H__
#define __DEV_SOUND_H__

#include <portaudio.h>
#include <pablio.h>

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
  
  int read(void *buf, int len); ///< reads audio data from the device in a buffer
  
  int write(void *buf, int len); ///< writes audio data from a buffer to the device

 private:
  PABLIO_Stream *aInStream; ///< Portaudio input stream 
  PABLIO_Stream *aOutStream; ///< Portaudio output stream 

  const PaDeviceInfo *info_input;
  const PaDeviceInfo *info_output;  

  PaError err;

};

#endif
