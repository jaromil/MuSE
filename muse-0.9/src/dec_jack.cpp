/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2004 Denis Roio aka jaromil <jaromil@dyne.org>
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
 *
 */

#include <dec_jack.h>
#include <jutils.h>
#include <config.h>

#ifdef HAVE_JACK

int jack_process(jack_nframes_t nframes, void *arg) {
  func("jack daemon callback to process %u frames",nframes);
  
  MuseDecJack *dec = (MuseDecJack*)arg;
  
  dec->jack_in_buf = (jack_default_audio_sample_t*)
    jack_port_get_buffer(dec->jack_in_port,nframes);
  
  dec->pipetta->write(dec->sample_size*nframes,
		      dec->jack_in_buf);

  return 0; // seems to be the success errorcode for jack
}

void jack_shutdown(void *arg) {
  func("jack daemon shutdown callback (TODO?)");
  MuseDecJack *dec = (MuseDecJack*)arg;
  dec->eos = true;
  jack_client_close(dec->client);
}

MuseDecJack::MuseDecJack() : MuseDec() {
  sample_size = sizeof(jack_default_audio_sample_t);
  pipetta = new Pipe(); // tune buffer size here
  pipetta->set_block(false,false);
  pipetta->set_output_type("copy_float_to_int32");
  
  strncpy(name,"Jack",5);
}

MuseDecJack::~MuseDecJack() {
  delete pipetta;
}

IN_DATATYPE *MuseDecJack::get_audio() {
  int res;
  // controlla se c'e' qualcosa nel pipe che e' provenuto dal callback
  // e ritorna l'informazione corretta (return code)

  if(eos) return NULL;

  err = false;

  res = pipetta->read(IN_CHUNK,_inbuf);
  if(res==IN_CHUNK) {
    warning("MuseDecJack::get_audio returns %u",res);
    frames = res;

  } else if(res<0) {
    warning("MuseDecJack::get_audio returns nothing from pipe");
    //    err = true;
    return NULL;

  } else {
    warning("MuseDecJack::get_audio buffer underrun");
    frames = res;
    return NULL;
  }

  return _inbuf;
}

int MuseDecJack::load(char *file) {
  if( (client = jack_client_new(file)) == 0 ) {
    error("can't open jack client. maybe the jack daemon is not running?");
    return 0; // return error
  }

  jack_set_process_callback(client, jack_process, this);

  samplerate = 44100; // jack_get_sample_rate(client);
  channels = 2;
  bitrate = 0;
  frametot = 0;
  seekable = false;
  eos = false;

  jack_on_shutdown(client, jack_shutdown, this);

  jack_in_port = jack_port_register(client, "input_channel",
				    JACK_DEFAULT_AUDIO_TYPE,
				    JackPortIsInput, 0);

  if(jack_activate(client)) {
    error("jack client opened, but unable to be activated");
    return 0; // error
  }
  
  return -2; // opened non seekable
}

bool MuseDecJack::seek(float pos) {
  func("MuseDecJack::seek(%.2f) : jack input channel can't seek!",pos);
  return false;
}


#endif // HAVE_JACK
