
/**
   @file Jack-audio connection daemon player
   @desc JackIt input channes: MuseDev implementation
*/

#ifndef __IN_JACK_H__
#define __IN_JACK_H__

#include <config.h>
#ifdef HAVE_JACK

#include <pipe.h>
#include <decoder.h>

extern "C" {
#include <jack/jack.h>
}

class MuseDecJack: public MuseDec {
 private:

  IN_DATATYPE _inbuf[IN_CHUNK+2];

 public:
  MuseDecJack();
  ~MuseDecJack();

  int load(char *file);
  bool seek(float pos);

  IN_DATATYPE *get_audio();

  jack_client_t *client;
  jack_port_t *jack_in_port;
  jack_default_audio_sample_t *jack_in_buf;
  size_t sample_size; // sizeof(jack_default_audio_sample_t);

  Pipe *pipetta;
};

#endif /* ifdef HAVE_JACK */
#endif
