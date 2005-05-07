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

#define DEFAULT_ENCODER OGG
#include <outchannels.h>
#include "carbon_message.h"
#include <jmixer.h>

class CarbonStreamEncoder {
	public:
		CarbonStreamEncoder(Stream_mixer *mix,CarbonMessage *cmsg);
		~CarbonStreamEncoder();
		bool saveToFile(char *fileName);
		void stopSaving();
				
		enum codec type(); /* get the encoder type */
		void type(enum codec t); /* set the encoder type */
		OutChannel *getOutChannel();
		
		int bitRate;
		int frequency;
		int channels;
		int lowpass;
		int highpass;
	private:
	
		void update();
	
		CarbonMessage *msg;
		enum codec _type;
		char *outFile;
		Stream_mixer *jmix;
		int encoderID;
};

#endif