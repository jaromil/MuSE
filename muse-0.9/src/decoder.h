
// "$Id$"

#ifndef __DECODER_H__
#define __DECODER_H__

#include <pthread.h>
#include <generic.h>

class MuseDec {
 public:

  MuseDec();
  virtual ~MuseDec();

  virtual int load(char *file) = 0; /* open filename */
  virtual bool seek(float pos) = 0; /* seek to position from 0.0 1.0 */
  virtual IN_DATATYPE *get_audio() = 0;  /* decode audio */

  char name[5];
  int samplerate;
  int channels;
  int bitrate;
  int frames;
  int framepos;
  int frametot;
  int fps;

  bool seekable;
  bool eos;
  bool err;

  /* pthread stuff */
  void lock() { pthread_mutex_lock(&mutex); };
  void unlock() { pthread_mutex_unlock(&mutex); };

 private:
  pthread_mutex_t mutex;

};    

#endif
