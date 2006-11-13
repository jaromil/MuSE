/* MPEG Sound library

   (C) 1997 by Woo-jae Jung */

// Soundinputstream.cc
// Abstractclass of inputstreams

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "mpegsound.h"

Soundinputstream::Soundinputstream()
{
  __errorcode=SOUND_ERROR_OK;
};

Soundinputstream::~Soundinputstream()
{
  // Nothing...
};

/********************/
/* File & Http open */
/********************/
Soundinputstream *Soundinputstream::hopen(char *filename,int *errorcode)
{
  if(!filename) return(NULL);

  Soundinputstream *st;

  if(strncmp(filename,"http://",7)==0) { /* network stream */
    st = new Soundinputstreamfromhttp;
    st->seekable = false;
  } else {
    st=new Soundinputstreamfromfile;
    st->seekable = true;
  }
  
  if(st==NULL) {
    *errorcode=SOUND_ERROR_MEMORYNOTENOUGH;
    return NULL;
  }

  if(!st->open(filename))
  {
    *errorcode=st->geterrorcode();
    delete st;
    return NULL;
  }
  
  return st;
}

