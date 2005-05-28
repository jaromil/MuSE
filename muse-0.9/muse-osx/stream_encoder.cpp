/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2005 xant <xant@dyne.org>
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
	outFile=NULL;
	/* DEFAULT ENCODER SETTINGS --- reflects nib control defaults */
	_bitrate=DEFAULT_BITRATE;
	_frequency=DEFAULT_FREQUENCY;
	_quality=DEFAULT_QUALITY;
	strcpy(_qdescr,DEFAULT_QUALITY_DESCR);
	_channels=DEFAULT_MODE; /* defaults to mono */
	type(DEFAULT_ENCODER);
	_filterMode=0;
	update();
}

CarbonStreamEncoder::~CarbonStreamEncoder() {
	OutChannel *enc = jmix->get_enc(encoderID);
	if(enc) jmix->delete_enc(encoderID);
}

bool CarbonStreamEncoder::update() {
	OutChannel *enc = jmix->get_enc(encoderID);
	if(enc) { /* encoder already exists */
		if(enc->tipo!=_type) { /* codec change!! */
			if(enc->icelist.len()) msg->warning(" Can't change encoder codec while server exists ... delete all servers before trying to change it.");
			else {
				jmix->delete_enc(encoderID);
				encoderID=jmix->create_enc(_type);
				enc=jmix->get_enc(encoderID);
			}
		}
	}
	else {
		encoderID=jmix->create_enc(_type);
		enc=jmix->get_enc(encoderID);
	}
	enc->bps(_bitrate);
	enc->freq(_frequency);
	enc->channels(_channels);
	enc->lowpass(_lowpass);
	enc->highpass(_highpass);
	bool res=jmix->apply_enc(encoderID);
//	if(!res) { /* if errors committing new encoder profile */ 
//		/* restore current values */
//		_bitrate=enc->bps();
//		_frequency=enc->freq();
//		_channels=enc->channels();
//		_lowpass=enc->lowpass();
//		_highpass=enc->highpass();
//	}
	if(!enc->running) enc->start();
	return res;
}

void CarbonStreamEncoder::type(enum codec t) {
	_type=t;
	//update();
}

OutChannel *CarbonStreamEncoder::getOutChannel() {
	return jmix->get_enc(encoderID);
}

void CarbonStreamEncoder::mode(int chans) {
	if(chans==1||chans==2) {
		_channels=chans;
	//	update();
	}
	else {
		warning("Bad value at CarbonStreamEncoder::mode() ... was %d .. should be 1 || 2",chans);
	}
}

int CarbonStreamEncoder::mode() {
	return _channels;
}

void CarbonStreamEncoder::bitrate(int bps) {
	_bitrate=bps;
}

void CarbonStreamEncoder::frequency(int freq) {
	_frequency=freq;
}
void CarbonStreamEncoder::lowpass(int filter) {
	_lowpass=filter;
}
void CarbonStreamEncoder::highpass(int filter) {
	_highpass=filter;
}

int CarbonStreamEncoder::bitrate() {
	return _bitrate;
}
int CarbonStreamEncoder::frequency() {
	return _frequency;
}
int CarbonStreamEncoder::lowpass() {
	return _lowpass;
}
int CarbonStreamEncoder::highpass() {
	return _highpass;
}

int CarbonStreamEncoder::quality() {
	return _quality;
}

enum codec CarbonStreamEncoder::type() {
	OutChannel *enc=getOutChannel();
	return enc->tipo;
}
/* just an accessor to OutChannel->quality()  [ 1 < q < 9 ]*/
char *CarbonStreamEncoder::quality(int q) {
	if(_quality!=q) {
		OutChannel *chan = getOutChannel();
		if(chan) {
			char *descr=chan->quality(((float)q));
		//	jmix->apply_enc(encoderID);
			_quality=q;
			strncpy(_qdescr,descr,255);
			_qdescr[255]=0;
			_bitrate=chan->bps();
			_frequency=chan->freq();
			return qualityString();
		}
	}
	return NULL;
}

char *CarbonStreamEncoder::qualityString() {
	return _qdescr;
}

void CarbonStreamEncoder::saveFile(char *fileName) {
	if(outFile) free(outFile);
	if(fileName) outFile=strdup(fileName);
	else outFile=NULL;
}

char *CarbonStreamEncoder::saveFile() {
	return outFile;
}
bool CarbonStreamEncoder::isSaving() {
	OutChannel *chan = getOutChannel();
	if(chan && chan->fd) return true;
	return false;
}

bool CarbonStreamEncoder::startSaving() {
	OutChannel *chan = getOutChannel();
	if(chan && outFile) {
		return chan->dump_start(outFile);
	}
	return false;
}

bool CarbonStreamEncoder::stopSaving() {
	OutChannel *chan = getOutChannel();
	if(chan) {
		return chan->dump_stop();
	}
	return false;
}

void CarbonStreamEncoder::filterMode(int mode) {
	if(mode>=0) 
		_filterMode=mode;
	if(mode==0) {
		highpass(0);
		lowpass(0);
	}
}

int CarbonStreamEncoder::filterMode() {
	return _filterMode;
}
