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

#include <config.h>

#define PA_SAMPLE_TYPE paInt16
#define PA_SAMPLES_PER_FRAME 2
#define PA_MAX_FRAMES 4096
#define PA_NUM_SECONDS 5

SoundDevice::SoundDevice() {
  aInStream = NULL;
  aOutStream = NULL;
}

SoundDevice::~SoundDevice() {
  close();
}

bool SoundDevice::input(bool state) {
  if(state && !aInStream) {
    err = OpenAudioStream( &aInStream, SAMPLE_RATE, PA_SAMPLE_TYPE,
			   (PABLIO_READ | PABLIO_STEREO) );
    if( err != paNoError) {
      Pa_Terminate();
      error("error opening input sound device: %s",Pa_GetErrorText( err ) );
      return false;
    }
    info_input = Pa_GetDeviceInfo( Pa_GetDefaultInputDeviceID() );
    func("input device: %s",info_input->name);

  } else if(!state && aInStream) {

    CloseAudioStream(aInStream);
    aInStream = NULL;

  }
  return true;
}

bool SoundDevice::output(bool state) {
  if(state && !aOutStream) {
    err = OpenAudioStream( &aOutStream, SAMPLE_RATE, PA_SAMPLE_TYPE,
			   (PABLIO_WRITE | PABLIO_STEREO) );
    if( err != paNoError) {
      Pa_Terminate();
      error("error opening output sound device: %s",Pa_GetErrorText( err ) );
      return false;
    }
    info_output = Pa_GetDeviceInfo( Pa_GetDefaultOutputDeviceID() );
    func("output device: %s",info_output->name);

  } else if(!state && aOutStream) {

    CloseAudioStream(aOutStream);
    aOutStream = NULL;

  }
  return true;
}

bool SoundDevice::open(bool read, bool write) {
  bool res;
  notice("detecting sound device");
  
  res = output(write);
  if(!res)
    return false;
  else
    act("output device: %s",info_output->name);  

  res = input(read);
  if(!res)
    return false;
  else 
    act("input device: %s",info_input->name);

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
  if(!aInStream) return -1;
  // len is in bytes, while portaudio takes samples
  return ReadAudioStream( aInStream, buf, len);
}

int SoundDevice::write(void *buf, int len) {
  if(!aOutStream) return -1;
  // len is in bytes, while portaudio takes samples
  return WriteAudioStream( aOutStream, buf, len);
}
