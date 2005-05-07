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
 
#include "stream_encoder.h"

/****************************************************************************/
/* CarbonStreamEncoder class */
/****************************************************************************/

CarbonStreamEncoder::CarbonStreamEncoder(Stream_mixer *mix,CarbonMessage *cmsg) {
	jmix=mix;
	msg=cmsg;
	encoderID=0;
	bitRate=16;
	frequency=22050;
	channels=1; /* defaults to mono */
	type(DEFAULT_ENCODER);
}

CarbonStreamEncoder::~CarbonStreamEncoder() {
	OutChannel *enc = jmix->get_enc(encoderID);
	if(enc) jmix->delete_enc(encoderID);
}

void CarbonStreamEncoder::update() {
	OutChannel *enc = jmix->get_enc(encoderID);
	if(enc) { /* encoder already exists */
		if(enc->tipo!=_type) { /* codec change!! */
			msg->warning(" KAKKA ");
		}
	}
	else {
		encoderID=jmix->create_enc(_type);
		enc=jmix->get_enc(encoderID);
	}
	enc->bps(bitRate);
	enc->freq(frequency);
	enc->channels(channels);
	enc->lowpass(lowpass);
	enc->highpass(highpass);
	jmix->apply_enc(encoderID);
}

void CarbonStreamEncoder::type(enum codec t) {
	_type=t;
	update();
}

OutChannel *CarbonStreamEncoder::getOutChannel() {
	return jmix->get_enc(encoderID);
}
