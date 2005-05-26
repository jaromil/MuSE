/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2002-2004 jaromil <jaromil@dyne.org>
 *
 * This sourcCARBONe code is free software; you can redistribute it and/or
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
 */

#include <Carbon/Carbon.h>

#ifndef __STREAM_ENCODER_H__
#define __STREAM_ENCODER_H__

#define CS_MONO 1
#define CS_STEREO 2

#include <outchannels.h>
#include "carbon_message.h"
#include <jmixer.h>

#define DEFAULT_BITRATE 16
#define DEFAULT_FREQUENCY 22050
#define DEFAULT_QUALITY 2
#define DEFAULT_MODE CS_MONO
#define DEFAULT_ENCODER OGG
#define DEFAULT_QUALITY_DESCR "16Kbit/s 22050Hz"

class CarbonStreamEncoder {
	public:
		CarbonStreamEncoder(Stream_mixer *mix,CarbonMessage *cmsg);
		~CarbonStreamEncoder();
						
		enum codec type(); /* get the encoder type */
		void type(enum codec t); /* set the encoder type */
		OutChannel *getOutChannel();
		
		/* set methods */
		void bitrate(int bps);
		void frequency(int freq);
		void mode(int mode);
		void lowpass(int filter);
		void highpass(int filter);
		char *quality(int q); // just an accessor to OutChannel->quality() 
		int quality();
		char *qualityString();
		void saveFile(char *fileName);
		void filterMode(int mode); /* 0 == auto, 1 == manual */
		
		bool startSaving();
		bool stopSaving();
		
		/* get methods */
		int bitrate();
		int frequency();
		int lowpass();
		int highpass();
		int filterMode(); /* 0 == auto , 1 == manual */
		int mode();
		char *saveFile();
		bool update();
		bool isSaving();
	private:
	
		CarbonMessage *msg;
		enum codec _type;
		char *outFile;
		Stream_mixer *jmix;
		int encoderID;

		int _bitrate;
		int _frequency;
		int _channels;
		int _lowpass;
		int _highpass;
		int _filterMode;
		int _quality;
		char _qdescr[256];
};

#endif