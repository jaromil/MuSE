
// "$Id$"

#include <decoder.h>
#include <jutils.h>
#include <config.h>

MuseDec::MuseDec() {
    bitrate = samplerate = channels = frames = 0;
    seekable = false; err = false; eos = false;
    if(pthread_mutex_init (&mutex,NULL) == -1)
      error("%i:%s error initializing POSIX thread mutex",
	    __LINE__,__FILE__);
}

MuseDec::~MuseDec() {
  if(pthread_mutex_destroy(&mutex) == -1)
    error("error destroying POSIX thread mutex",
	  __LINE__,__FILE__);
}
