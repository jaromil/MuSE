/* MuSE - Multiple Streaming Engine
 * SoundDevice class interfacing Portaudio PABLIO library
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
 
 "$Id$"
 
 */

#include <dev_sound.h>
#include <pablio.h>
#include <jutils.h>
#include <generic.h>

#define PA_SAMPLE_TYPE paInt16
#define PA_SAMPLES_PER_FRAME 2
#define PA_MAX_FRAMES 4096
#define PA_NUM_SECONDS 5

int dev_jack_process(jack_nframes_t nframes, void *arg) {
  jack_nframes_t opframes;
  SoundDevice *dev = (SoundDevice*)arg;
  if(!dev->jack) return 0; // just return
  
  // take output from pipe and send it to jack
  dev->jack_out_buf = (jack_default_audio_sample_t*)
    jack_port_get_buffer(dev->jack_out_port,nframes);
  opframes = dev->jack_out_pipe->read
    (nframes * sizeof(float) * 2 , dev->jack_out_buf);

  // take input from jack and send it in pipe
  dev->jack_in_buf = (jack_default_audio_sample_t*)
    jack_port_get_buffer(dev->jack_in_port,nframes);
  dev->jack_in_pipe->write // does the float2int conversion in one pass
    (nframes * sizeof(float) * 2 , dev->jack_in_buf);

  return 0;
}

void dev_jack_shutdown(void *arg) {
  SoundDevice *dev = (SoundDevice*)arg;
  // close the jack channels
  dev->jack = false;
  jack_port_unregister(dev->jack_client, dev->jack_in_port);
  jack_port_unregister(dev->jack_client, dev->jack_out_port);
  jack_deactivate(dev->jack_client);
  delete dev->jack_in_pipe;
  delete dev->jack_out_pipe;
}

SoundDevice::SoundDevice() {
  aInStream = NULL;
  aOutStream = NULL;
  jack = false;
  jack_in = false;
  jack_out = false;
}

SoundDevice::~SoundDevice() {
  close();
}

bool SoundDevice::pablio_input(bool state) {
  if(state && !aInStream) {
    err = OpenAudioStream( &aInStream, SAMPLE_RATE, PA_SAMPLE_TYPE,
			   (PABLIO_READ | PABLIO_STEREO) );
    if( err != paNoError) {
      Pa_Terminate();
      error("error opening input sound device: %s",Pa_GetErrorText( err ) );
      return false;
    } else
      info_input = Pa_GetDeviceInfo( Pa_GetDefaultInputDeviceID() );

  } else if(!state && aInStream) {

    CloseAudioStream(aInStream);
    aInStream = NULL;
    info_input = NULL;

  }
  return true;
}

bool SoundDevice::input(bool state) {
  bool res = false;
  if(jack) return true;
  if(!res) res = pablio_input(state);
  return res;
}


bool SoundDevice::pablio_output(bool state) {
  if(state && !aOutStream) {
    err = OpenAudioStream( &aOutStream, SAMPLE_RATE, PA_SAMPLE_TYPE,
			   (PABLIO_WRITE | PABLIO_STEREO) );
    if( err != paNoError) {
      Pa_Terminate();
      error("error opening output sound device: %s",Pa_GetErrorText( err ) );
      return false;
    } else
      info_output = Pa_GetDeviceInfo( Pa_GetDefaultOutputDeviceID() );

  } else if(!state && aOutStream) {
    
    CloseAudioStream(aOutStream);
    aOutStream = NULL;
    info_output = NULL;

  }
  return true;
}

bool SoundDevice::output(bool state) {
  bool res = false;
  if(jack) return true;
  if(!res) res = pablio_output(state);
  return res;
}

bool SoundDevice::open(bool read, bool write) {

  //  notice("detecting sound device");

#ifdef HAVE_JACK
  // we try to open up a jack client
  jack_sample_size = sizeof(jack_default_audio_sample_t);
  if(!jack) // check if it is not allready on
    if( (jack_client = jack_client_new("MuSE")) !=0 ) {
      notice("jack audio daemon detected");
      act("hooking in/out sound channels");
      warning("this feature is still experimental and won't work!");
      warning("you need to stop jack and free the audio card");
      jack = true;
      jack_samplerate = jack_get_sample_rate(jack_client);
      jack_set_process_callback(jack_client, dev_jack_process, this);    
      jack_on_shutdown(jack_client,dev_jack_shutdown,this);

      jack_in_pipe = new Pipe();
      jack_in_pipe->set_output_type("copy_float_to_int16");
      jack_in_pipe->set_block(false,true);

      jack_out_pipe = new Pipe();
      jack_out_pipe->set_input_type("copy_int16_to_float");
      jack_in_pipe->set_block(true,false);

      // open the jack input channel
      jack_in_port = jack_port_register(jack_client, "capture",
					JACK_DEFAULT_AUDIO_TYPE,
					JackPortIsInput, 0);
      // open the jack output channel
      jack_out_port = jack_port_register(jack_client, "playback",
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsOutput, 0);
      
      jack_activate(jack_client);
      return true;
    }
#endif
  
  if( ! output(write) ) return false;
  
  if(info_output) func("output device: %s",info_output->name);
  else error("output device not available");
  
  if( ! input(read) ) return false;
  
  if(info_input) func("input device: %s",info_input->name);
  else error("input device not available");

  return true;
}

void SoundDevice::close() {
  if(aInStream)
    CloseAudioStream( aInStream);
  aInStream = NULL;

  if(aOutStream)
    CloseAudioStream( aOutStream);
  aOutStream = NULL;
}

int SoundDevice::read(void *buf, int len) {
  // len is in samples: 4*2 32bit stereo
  int res = -1;

  if(jack) {

    res = jack_in_pipe->read(len<<1,buf);

  } else if(aInStream) { // pablio

    // takes number of left and right frames (stereo / 2)
    res = ReadAudioStream( aInStream, buf, len);

  }  
  return res;
}

int SoundDevice::write(void *buf, int len) {
  // len is in samples, for bytes must *2 (16bit)
  int res = -1;

  if(jack) { // jack audio daemon

    res = jack_out_pipe->write(len<<1,buf);

  } else if(aOutStream) { // pablio

    // takes number of left and right frames (stereo / 2)
    res = WriteAudioStream( aOutStream, buf, len>>1);

  }
  return res;

}
