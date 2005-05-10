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

#include "carbon_stream.h"
#include <jutils.h>

extern "C" OSStatus OpenFolderWindow(WindowRef parent);
void qualityHandler (ControlRef theControl, ControlPartCode partCode);

/****************************************************************************/
/* CarbonStream globals */
/****************************************************************************/

const ControlID streamTabID = { CARBON_GUI_APP_SIGNATURE, STREAM_TAB_CONTROL };
const ControlID serverTabID = { CARBON_GUI_APP_SIGNATURE, SERVER_TAB_CONTROL };

#define STREAM_EVENTS 3
const EventTypeSpec windowEvents[] = {
	{ kEventClassWindow, kEventWindowActivated },
	{ kEventClassWindow, kEventWindowClose },
	{ kCoreEventClass, kAEOpenDocuments }
};

const EventTypeSpec streamCommands[] = {
	{ kEventClassCommand, kEventCommandProcess }
};

static const EventTypeSpec tabControlEvents[] = {
    {kEventClassControl, kEventControlHit} 
};


static OSStatus StreamEventHandler (
	EventHandlerCallRef nextHandler, EventRef event, void *userData);
	
static OSStatus streamCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);

OSStatus StreamTabEventHandler(EventHandlerCallRef inCallRef, 
	EventRef inEvent, void* inUserData );
	
OSStatus ServerTabEventHandler(EventHandlerCallRef inCallRef, 
	EventRef inEvent, void* inUserData );

#define CS_ALLOWED_BPS_NUM 8
#define CS_ALLOWED_FREQ_NUM 4
const int bps[CS_ALLOWED_BPS_NUM] = { 16,24,32,48,56,64,96,128 };
const int freq[CS_ALLOWED_FREQ_NUM] = { 11000,16000,22050,44100 };

/****************************************************************************/
/* CarbonStream class */
/****************************************************************************/

CarbonStream::CarbonStream(Stream_mixer *mix,WindowRef mainWin,IBNibRef nib) {
		parent=mainWin;
		jmix=mix;
		nibRef=nib;
		OSStatus err;
		
		_selectedStream = 0;
		_selectedServer = 0;
		
		memset(enc,0,sizeof(enc));
		memset(servers,0,sizeof(servers)); 
		msg = new CarbonMessage(nibRef);
		err = CreateWindowFromNib(nibRef,CFSTR("StreamWindow"),&window);
		if(err != noErr) { 
			msg->error("Can't create the stream configuration window (%d)!!",err);
		}
		err = InstallEventHandler(GetWindowEventTarget(window),StreamEventHandler,STREAM_EVENTS,windowEvents,this,NULL);
		if(err != noErr) { 
			msg->error("Can't install event handler for Channel control (%d)!!",err);
		}
		/* install the stream command handler */
		err = InstallWindowEventHandler (window, 
            NewEventHandlerUPP (streamCommandHandler), 
            GetEventTypeCount(streamCommands), streamCommands, 
            this, NULL);
		if(err != noErr) msg->error("Can't install stream commandHandler");
		
		/* setup stream tab control */
		err=GetControlByID(window,&streamTabID,&streamTabControl);
		if(err!=noErr) msg->error("Can't get streamTabControl (%d)!!",err);
		err=InstallControlEventHandler (streamTabControl,NewEventHandlerUPP(StreamTabEventHandler),
			GetEventTypeCount(tabControlEvents),tabControlEvents,this, NULL); 
		if(err!=noErr) msg->error("Can't install streamTab event handler (%d)!!",err);
		HideControl(streamTabControl);
		_selectedStream=0;
		
		/* setup server tab control */
		err=GetControlByID(window,&serverTabID,&serverTabControl);
		if(err!=noErr) msg->error("Can't get serverTabControl (%d)!!",err);
		err=InstallControlEventHandler (serverTabControl,NewEventHandlerUPP(ServerTabEventHandler),
			GetEventTypeCount(tabControlEvents),tabControlEvents,this, NULL); 
		if(err!=noErr) msg->error("Can't install serverTab event handler (%d)!!",err);
		HideControl(serverTabControl);
		_selectedServer=0;
		
		/* install and handler to allow live quality change */
		ControlRef qualityControl;
		const ControlID qualityID = { CARBON_GUI_APP_SIGNATURE, QUALITY_CONTROL };
		err=GetControlByID(window,&qualityID,&qualityControl);
		if(err!=noErr) msg->error("Can't get quality control (%d)!!",err);
		CarbonStream *self=this;
		err = SetControlProperty(qualityControl,CARBON_GUI_APP_SIGNATURE,QUALITY_PROPERTY,
			sizeof(CarbonStream *),&self);
		if(err!=noErr) msg->error("Can't attach CarbonChannel object to Fader control (%d) !!",err);
		SetControlAction(qualityControl,NewControlActionUPP(&qualityHandler));
}

CarbonStream::~CarbonStream() {
	DisposeWindow(window);
}

void CarbonStream::show() {
	RepositionWindow(window,parent,kWindowCenterOnMainScreen);
	ShowWindow(window);
	BringToFront(window);
	ActivateWindow(window,true);
}

void CarbonStream::hide() {
	ActivateWindow(window,false);
	HideWindow(window);
	ActivateWindow(parent,true);
}


#define SELECT_TAB_CONTROL(__cid,__control) \
	{\
		switch(__cid) {\
			case STREAM_TAB_CONTROL:\
				__control = &streamTabControl;\
				break;\
			case SERVER_TAB_CONTROL:\
				__control = &serverTabControl;\
				break;\
			default:\
				break;\
		}\
	}

int CarbonStream::nextTabIndex(SInt32 controlID) {
	ControlRef *control;
	SELECT_TAB_CONTROL(controlID,control);
	SInt32 nVal=GetControl32BitMaximum(*control);
	int max,i;
	if(controlID==STREAM_TAB_CONTROL) max = MAX_STREAM_ENCODERS;
	else if(controlID==SERVER_TAB_CONTROL) max = MAX_STREAM_SERVERS;
	else return -1;
	
	int check[max];
	memset(check,0,sizeof(check));
	int newIdx=0;
	for (i=1;i<=nVal;i++) {
			int tVal=getTabValue(controlID,i);
			check[tVal-1]=1;
	}
	for(i=0;i<max;i++) {
		if(!check[i]) {
			return i+1;
		}
	}
	return -1;
}
	
bool CarbonStream::addTab(SInt32 controlID) {
	int max;
	ControlRef *control;
	SELECT_TAB_CONTROL(controlID,control);
	SInt32 val = GetControl32BitMaximum(*control);
	val++;
	
	if(controlID==STREAM_TAB_CONTROL) max = MAX_STREAM_ENCODERS;
	else if(controlID==SERVER_TAB_CONTROL) max = MAX_STREAM_SERVERS;
	else return false;
	
	if(val>max) {
		msg->warning("MAX TABS!!");
		//SetControl32BitMaximum(*control,val-1);
		return false;
	}

	int newIdx=nextTabIndex(controlID);
	if(!newIdx) return false;
	SetControl32BitMaximum(*control,val);
	SetControl32BitValue(*control,val);
	setTabValue(controlID,val,newIdx);
	return true;
}

void CarbonStream::delTab(SInt32 controlID,unsigned int idx) {
	ControlRef *control;
	SELECT_TAB_CONTROL(controlID,control);
	SInt32 sNum=GetControl32BitMaximum(*control);
	if(sNum > 1) {
		SInt32 selected = GetControl32BitValue(*control);
	//	if(selected==idx) SetControl32BitValue(*control,selected>1?(selected-1):(selected+1));
		for(unsigned int i=idx;i<sNum;i++) {
			struct ControlTabInfoRec info;
			memset(&info,0,sizeof(struct ControlTabInfoRec));
			OSErr err = GetControlData(*control,i+1,kControlTabInfoTag,sizeof(struct ControlTabInfoRec),&info,NULL);
			if(err!=noErr) {
				msg->error("Can't get tab value (%d)!!",err);
			}
			err=SetControlData(*control,i,kControlTabInfoTag,sizeof(struct ControlTabInfoRec),&info);
			if(err!=noErr) msg->error("Can't set tab value (%d)!!",err);
		}
		SetControl32BitMaximum(*control,sNum-1);
	}
	else {
		HideControl(*control);
		if(controlID==STREAM_TAB_CONTROL) _selectedStream=0;
		else _selectedServer=0;
	}
}

int CarbonStream::getTabValue(SInt32 controlID,int tabIndex) {
	ControlRef *control;
	SELECT_TAB_CONTROL(controlID,control);
	int lVal=0;
	struct ControlTabInfoRec info;
	if(!tabIndex) return 0;
	memset(&info,0,sizeof(struct ControlTabInfoRec));
	OSErr err = GetControlData(*control,tabIndex,kControlTabInfoTag,sizeof(struct ControlTabInfoRec),&info,NULL);
	if(err!=noErr) {
		msg->warning("Can't get tab value (%d)!!",err);
		return 0;
	}
	//sprintf((char *)info.name," %d",val);
	sscanf((char *)info.name,"%d",&lVal);
	return lVal;
}

void CarbonStream::setTabValue(SInt32 controlID,int tabIndex,int val) {
	ControlRef *control;
	SELECT_TAB_CONTROL(controlID,control);
	struct ControlTabInfoRec info;
	memset(&info,0,sizeof(struct ControlTabInfoRec));
	sprintf((char *)info.name," %d",val);
	OSErr err=SetControlData(*control,tabIndex,kControlTabInfoTag,sizeof(struct ControlTabInfoRec),&info);
	if(err!=noErr) msg->warning("%d",err);
}

void CarbonStream::addStream() {
	SInt32 sNum=GetControl32BitMaximum(streamTabControl);
	if(!IsControlVisible(streamTabControl)) { /* First stream */ 
		if(enc[0]) {
			delete enc[0];
		}
		enc[0] = new CarbonStreamEncoder(jmix,msg);
		if(!enc[0]) {
			/* XXX - errors here */
			return;
		}
		setTabValue(STREAM_TAB_CONTROL,1,1);
		ShowControl(streamTabControl);
	//	EnableControl(streamTabControl);
	
		updateStreamTab();
	}
	else if(addTab(STREAM_TAB_CONTROL)) { /* new tab must be created */
		int encIdx = getTabValue(STREAM_TAB_CONTROL,sNum+1)-1;
		if(enc[encIdx]) {
			delete enc[encIdx];
		}
		enc[encIdx]=new CarbonStreamEncoder(jmix,msg);
		if(!enc[encIdx]) {
			/* XXX - errors here */
			return;
		}
		updateStreamTab();
	}
}

void CarbonStream::deleteStream(unsigned int idx) {
	if(idx > MAX_STREAM_ENCODERS) {
		/* ERRORS HERE */
		return;
	} 
	int encIdx=getTabValue(STREAM_TAB_CONTROL,idx)-1;
	int numServers=GetControl32BitMaximum(serverTabControl);
	if(enc[encIdx]) {
		//for (int i=0;i<MAX_STREAM_SERVERS;i++) {
		//	if(servers[encIdx][i]) {
		//		deleteServer(idx,i+1);
		//	}
		//}
		if(IsControlVisible(serverTabControl)) {
			for(int i=numServers;i>0;i--) {
				deleteServer(idx,i);
			}
		}
		delete enc[encIdx];
		enc[encIdx]=NULL;
	}
	delTab(STREAM_TAB_CONTROL,idx);
	if(!updateStreamTab()) changeServerTab(); 
}

void CarbonStream::deleteStream() {
	SInt32 selected=GetControl32BitValue(streamTabControl);
	deleteStream(selected);
}

void CarbonStream::deleteServer() {
	SInt32 selectedStream=GetControl32BitValue(streamTabControl);
	SInt32 selectedServer=GetControl32BitValue(serverTabControl);
	deleteServer(selectedStream,selectedServer);
}

void CarbonStream::deleteServer(unsigned int streamIndex,unsigned int serverIndex) {
	int encIdx=getTabValue(STREAM_TAB_CONTROL,streamIndex)-1;
	int serIdx=getTabValue(SERVER_TAB_CONTROL,serverIndex)-1;
	if(serIdx <0 || encIdx < 0) return;
	if(servers[encIdx][serIdx]) {
		delete servers[encIdx][serIdx];
		servers[encIdx][serIdx]=NULL;
		delTab(SERVER_TAB_CONTROL,serverIndex);
		if(IsControlVisible(serverTabControl)) {
			SInt32 newSel = GetControl32BitValue(serverTabControl);
			int newIdx=getTabValue(SERVER_TAB_CONTROL,newSel)-1;
			if(servers[encIdx][newIdx]) 
				updateServerInfo(servers[encIdx][newIdx]);
		}
	}
}

void CarbonStream::addServer() {
	SInt32 sNum=GetControl32BitMaximum(serverTabControl);
	int selectedStream = getTabValue(STREAM_TAB_CONTROL,GetControl32BitValue(streamTabControl))-1;
	CarbonStreamEncoder *sEnc = enc[selectedStream];
	if(!sEnc) {
		msg->warning("Can't get encoder object for the selected stream !!");
		return;
	}
	if(!IsControlVisible(serverTabControl)) { /* First server */ 
		servers[selectedStream][0]=new CarbonStreamServer(sEnc);
		setTabValue(SERVER_TAB_CONTROL,1,1);
		ShowControl(serverTabControl);
	//	EnableControl(serverTabControl);
		
		updateServerTab();
	}
	else { /* new tab must be created */
		if(addTab(SERVER_TAB_CONTROL)) {
			int serverIndex=getTabValue(SERVER_TAB_CONTROL,sNum+1)-1;
			if(servers[selectedStream][serverIndex]) {
				/* XXX - error messages here */
				delete servers[selectedStream][serverIndex];
			}
			servers[selectedStream][serverIndex]=new CarbonStreamServer(sEnc);
			if(!updateServerTab())
				updateServerInfo(servers[selectedStream][serverIndex]);
		}
	}
}

bool CarbonStream::updateStreamTab() {
	SInt32 val = GetControl32BitValue(streamTabControl);
	if(_selectedStream==val) return false; /* no changes ... */
	changeServerTab();
	_selectedStream=val;
	updateStreamInfo(selectedStream());
	return true;
}

bool CarbonStream::updateServerTab() {
	SInt32 val = GetControl32BitValue(serverTabControl);
	SInt32 max = GetControl32BitMaximum(serverTabControl);
	int streamIndex=getTabValue(STREAM_TAB_CONTROL,_selectedStream)-1;
	if(_selectedServer>max) _selectedServer=max;
	else if(_selectedServer==val) return false;
	if(_selectedServer && IsControlVisible(serverTabControl)) { 
		int serverIndex=getTabValue(SERVER_TAB_CONTROL,_selectedServer)-1;
		saveServerInfo(servers[streamIndex][serverIndex]);
	}
	_selectedServer=val;
	int newServerIndex=getTabValue(SERVER_TAB_CONTROL,_selectedServer)-1;
	updateServerInfo(servers[streamIndex][newServerIndex]);
	return true;
}

bool CarbonStream::changeServerTab() {
	SInt32 val = GetControl32BitValue(streamTabControl);
	int newStreamIndex=getTabValue(STREAM_TAB_CONTROL,val)-1;
	int oldServerIndex=getTabValue(SERVER_TAB_CONTROL,_selectedServer)-1;
	int sNum=0;
	_selectedServer=1;
	if(IsControlVisible(serverTabControl)) {
		int oldStreamIndex=getTabValue(STREAM_TAB_CONTROL,_selectedStream)-1;
		saveServerInfo(servers[oldStreamIndex][oldServerIndex]);
	}
	SetControl32BitMaximum(serverTabControl,1);
	HideControl(serverTabControl);
	_selectedServer=0;
	for(int i=0;i<MAX_STREAM_SERVERS;i++) {
		if(servers[newStreamIndex][i]) {
			sNum++;
			if(!IsControlVisible(serverTabControl)) ShowControl(serverTabControl);
			SetControl32BitMaximum(serverTabControl,sNum);
			setTabValue(SERVER_TAB_CONTROL,sNum,i+1);
		}
	}
	SetControl32BitValue(serverTabControl,1);
	if(IsControlVisible(serverTabControl)) {
		int newServerIndex=getTabValue(SERVER_TAB_CONTROL,1)-1;
		updateServerInfo(servers[newStreamIndex][newServerIndex]);
	}
}

#define SAVE_SERVER_INFO(__id,__name) \
	{\
		cid.id=__id;\
		err=GetControlByID(window,&cid,&control);\
		err=GetControlData(control,0,kControlEditTextTextTag,SERVER_STRING_BUFFER_LEN-1,buffer,NULL);\
		if(err!=noErr) msg->error("Can't get %s from text control (%d)!!","__name",err);\
	}
#define SAVE_SERVER_TEXT_INFO(__id,__name,__func) \
	{\
		SAVE_SERVER_INFO(__id,__name) \
		server->__func(buffer);\
		memset(buffer,0,sizeof(buffer));\
	}
#define SAVE_SERVER_INT_INFO(__id,__name,__func) \
	{\
		SAVE_SERVER_INFO(__id,__name) \
		intBuffer=0;\
		sscanf(buffer,"%d",&intBuffer);\
		server->__func(intBuffer);\
		intBuffer=0;\
		memset(buffer,0,sizeof(buffer));\
	}
	
void CarbonStream::saveServerInfo(CarbonStreamServer *server) {
	ControlRef control;	
	OSStatus err;
	int intBuffer =0;
	if(!server) return;
	char buffer[SERVER_STRING_BUFFER_LEN];
	ControlID cid;

	cid.signature=CARBON_GUI_APP_SIGNATURE;
	// memset(buffer,0,sizeof(buffer)); // should be useless
	
	/* port */
	SAVE_SERVER_INT_INFO(PORT_CONTROL,port,port);
	/* login type */
	//SAVE_SERVER_INT_INFO(server	
	/* host */
	SAVE_SERVER_TEXT_INFO(HOST_CONTROL,host,host);
	/* mnt */
	SAVE_SERVER_TEXT_INFO(MNT_CONTROL,mnt,mount);
	/* name */
	SAVE_SERVER_TEXT_INFO(NAME_CONTROL,name,name);
	/* url */
	SAVE_SERVER_TEXT_INFO(URL_CONTROL,url,url);
	/*description */
	SAVE_SERVER_TEXT_INFO(DESCRIPTION_CONTROL,description,description);
	/* username */
	SAVE_SERVER_TEXT_INFO(USERNAME_CONTROL,username,username);
	/* password */
	SAVE_SERVER_TEXT_INFO(PASSWORD_CONTROL,password,password);
}

#define UPDATE_SERVER_TEXT_INFO(__id,__name,__func) \
	{\
		cid.id=__id;\
		err=GetControlByID(window,&cid,&control);\
		buffer = server->__func();\
		len=buffer?strlen(buffer):0;\
		err=SetControlData(control,0,(__id==PASSWORD_CONTROL)?kControlEditTextPasswordTag:\
			kControlEditTextTextTag,len,buffer);\
		if(err!=noErr) msg->warning("Can't set %s for text control (%d)!!","__name",err);\
		buffer=NULL;\
	}

#define UPDATE_SERVER_INT_INFO(__id,__name,__func) \
	{\
		cid.id=__id;\
		err=GetControlByID(window,&cid,&control);\
		if(err!=noErr) msg->error("Can't get control (%d)!!",err);\
		intBuf = server->__func();\
		if(intBuf) sprintf(intBufStr,"%d",intBuf);\
		else *intBufStr=0;\
		len=strlen(intBufStr);\
		err=SetControlData(control,0,kControlEditTextTextTag,len,intBufStr);\
		if(err!=noErr) msg->warning("Can't set %s from text control (%d)!!","__name",err);\
		intBuf=0;\
		*intBufStr=0;\
	}
	
void CarbonStream::updateServerInfo(CarbonStreamServer *server) {
	ControlRef control;
	ControlID cid;
	int len;
	OSStatus err;
	char *buffer=NULL;
	int intBuf=0;
	char intBufStr[256];
	cid.signature=CARBON_GUI_APP_SIGNATURE;

	/* host */
	UPDATE_SERVER_TEXT_INFO(HOST_CONTROL,host,host);
	/* mnt */
	UPDATE_SERVER_TEXT_INFO(MNT_CONTROL,mnt,mount);
	/* name */
	UPDATE_SERVER_TEXT_INFO(NAME_CONTROL,name,name);
	/* url */
	UPDATE_SERVER_TEXT_INFO(URL_CONTROL,url,url);
	/*description */
	UPDATE_SERVER_TEXT_INFO(DESCRIPTION_CONTROL,description,description);
	/* username */
	UPDATE_SERVER_TEXT_INFO(USERNAME_CONTROL,username,username);
	/* password */
	UPDATE_SERVER_TEXT_INFO(PASSWORD_CONTROL,password,password);
	
	/*port */
	UPDATE_SERVER_INT_INFO(PORT_CONTROL,port,port);
	
	/* connect button */
	ControlID cbutID={ CARBON_GUI_APP_SIGNATURE, CONNECT_BUTTON };
	cid.id=CONNECT_BUTTON;
	err=GetControlByID(window,&cid,&control);
	if(err!=noErr) msg->error("Can't get connect button control (%d)!!",err);
	if(server->isConnected()) {
		SetControlTitleWithCFString(control,CFSTR("Disconnect"));
	}
	else {
		SetControlTitleWithCFString(control,CFSTR("Connect"));
	}
}

bool CarbonStream::doConnect() {
	CarbonStreamServer *server=selectedServer();
	saveServerInfo(server);
	ControlID cbutID={ CARBON_GUI_APP_SIGNATURE, CONNECT_BUTTON };
	ControlRef cbutControl;
	OSStatus err=GetControlByID(window,&cbutID,&cbutControl);
	if(err!=noErr) msg->error("Can't get connect button control (%d)!!",err);

	if(server->isConnected() && disconnectServer(server)) {
		SetControlTitleWithCFString(cbutControl,CFSTR("Connect"));
	}
	else if(connectServer(server)) {
		SetControlTitleWithCFString(cbutControl,CFSTR("Disconnect"));
	}
}

bool CarbonStream::disconnectServer(CarbonStreamServer *server) {
	if(server) return server->disconnect();
}

bool CarbonStream::connectServer(CarbonStreamServer *server) {
	if(server) return server->connect();
}

CarbonStreamServer *CarbonStream::selectedServer() {
	SInt32 selectedStream=GetControl32BitValue(streamTabControl);
	SInt32 selectedServer=GetControl32BitValue(serverTabControl);
	int encIdx=getTabValue(STREAM_TAB_CONTROL,selectedStream)-1;
	int serIdx=getTabValue(SERVER_TAB_CONTROL,selectedServer)-1;
	return servers[encIdx][serIdx];
}

CarbonStreamEncoder *CarbonStream::selectedStream() {
	SInt32 selectedStream=GetControl32BitValue(streamTabControl);
	int encIdx=getTabValue(STREAM_TAB_CONTROL,selectedStream)-1;
	return enc[encIdx];
}

void CarbonStream::updateQuality() {
	ControlRef qualityControl,descrControl;
	OSStatus err;
	const ControlID qualityID = { CARBON_GUI_APP_SIGNATURE, QUALITY_CONTROL };
	const ControlID descrID = { CARBON_GUI_APP_SIGNATURE, STREAM_DESCR_CONTROL };
	CarbonStreamEncoder *enc = selectedStream();
	
	/* get controls */
	err=GetControlByID(window,&qualityID,&qualityControl);
	if(err!=noErr) msg->error("Can't get quality control (%d)!!",err);
	err=GetControlByID(window,&descrID,&descrControl);
	if(err!=noErr) msg->error("Can't get quality descr control (%d)!!",err);
	
	/* get selected value */
	SInt32 quality = GetControlValue(qualityControl);
	enc->quality(quality);
	updateStreamInfo(enc);
}

void CarbonStream::updateBitrate() {
	CarbonStreamEncoder *enc=selectedStream();
	ControlRef bpsControl;
	const ControlID bpsID = { CARBON_GUI_APP_SIGNATURE,BITRATE_CONTROL };
	OSStatus err=GetControlByID(window,&bpsID,&bpsControl);
	if(err!=noErr) msg->error("Can't get bps control (%d)!!",err);
	SInt32 val = GetControlValue(bpsControl);
	enc->bitrate(bps[val-1]);
}

void CarbonStream::updateFrequency() {
	CarbonStreamEncoder *enc=selectedStream();
	ControlRef freqControl;
	const ControlID freqID = { CARBON_GUI_APP_SIGNATURE,FREQUENCY_CONTROL };
	OSStatus err=GetControlByID(window,&freqID,&freqControl);
	if(err!=noErr) msg->error("Can't get frequency control (%d)!!",err);
	SInt32 val = GetControlValue(freqControl);
	enc->frequency(freq[val-1]);
}

void CarbonStream::updateMode() {
	CarbonStreamEncoder *enc=selectedStream();
	ControlRef modeControl;
	const ControlID modeID = { CARBON_GUI_APP_SIGNATURE,MODE_CONTROL };
	OSStatus err=GetControlByID(window,&modeID,&modeControl);
	if(err!=noErr) msg->error("Can't get mode control (%d)!!",err);
	SInt32 val = GetControlValue(modeControl);
	enc->mode(val);
}

void CarbonStream::recordStreamPath(char *path) {
	ControlRef recordNameControl;
	SInt32 selectedStream=GetControl32BitValue(streamTabControl);
	int encIdx=getTabValue(STREAM_TAB_CONTROL,selectedStream)-1;
	const ControlID recordNameID = { CARBON_GUI_APP_SIGNATURE, RECORD_STREAM_FILENAME };
	OSStatus err=GetControlByID(window,&recordNameID,&recordNameControl);
	if(err!=noErr) msg->error("Can't get recordName text control (%d)!!",err);
	char *outFilename = (char *)malloc(strlen(path)+12); /* XXX - HC LEN */
	sprintf(outFilename,"%s/Stream_%d",path,selectedStream);
	err=SetControlData(recordNameControl,0,kControlEditTextTextTag,strlen(outFilename),outFilename);
	if(err!=noErr) msg->warning("Can't set record filename control (%d)!!",err);
	free(outFilename);
}

void CarbonStream::recordStream(bool on) {
}

void CarbonStream::codecChange() {
	CarbonStreamEncoder *enc=selectedStream();
	ControlRef typeControl;
	const ControlID typeID = { CARBON_GUI_APP_SIGNATURE,ENCODER_SELECT_CONTROL };
	OSStatus err=GetControlByID(window,&typeID,&typeControl);
	if(err!=noErr) msg->error("Can't get type control (%d)!!",err);
	SInt32 val = GetControlValue(typeControl);
	enc->type((val==1)?OGG:MP3);
	updateStreamInfo(enc);
}

void CarbonStream::updateStreamInfo(CarbonStreamEncoder *encoder) {
	int i;
	OSStatus err;
	ControlRef control;
	ControlID cid={CARBON_GUI_APP_SIGNATURE,0};
	if(!encoder) return;
	
	/* encoder type */
	cid.id=ENCODER_SELECT_CONTROL;
	err=GetControlByID(window,&cid,&control);
	if(err!=noErr) msg->error("Can't get type control (%d)!!",err);
	SetControlValue(control,(encoder->type()==OGG)?1:2);

	/* quality */
	cid.id=QUALITY_CONTROL;
	err=GetControlByID(window,&cid,&control);
	if(err!=noErr) msg->error("Can't get quality control (%d)!!",err);
	SetControlValue(control,encoder->quality());
	
	/* quality descr */
	cid.id=STREAM_DESCR_CONTROL;
	err=GetControlByID(window,&cid,&control);
	if(err!=noErr) msg->error("Can't get qualityString control (%d)!!",err);
	char *descr=encoder->qualityString();
	if(descr) {
		CFStringRef sdescr = CFStringCreateWithCString(NULL,descr,kCFStringEncodingMacRoman);
		SetControlData(control,0,kControlStaticTextCFStringTag,sizeof(CFStringRef),&sdescr);
		CFRelease(sdescr);	
	}
	
	/* bitrate */
	cid.id=BITRATE_CONTROL;
	err=GetControlByID(window,&cid,&control);
	if(err!=noErr) msg->error("Can't get bps control (%d)!!",err);
	for (i=0;i<CS_ALLOWED_BPS_NUM;i++) {
		if(bps[i]==encoder->bitrate()) {
			SetControlValue(control,i+1);
			break;
		}
	}	
	
	/* frequency */
	cid.id=FREQUENCY_CONTROL;
	err=GetControlByID(window,&cid,&control);
	if(err!=noErr) msg->error("Can't get frequency control (%d)!!",err);
	for (i=0;i<CS_ALLOWED_FREQ_NUM;i++) {
		if(freq[i]==encoder->frequency()) {
			SetControlValue(control,i+1);
			break;
		}
	}
	
	/* channels (stereo/mono) */
	cid.id=MODE_CONTROL;
	err=GetControlByID(window,&cid,&control);
	if(err!=noErr) msg->error("Can't get mode control (%d)!!",err);
	SetControlValue(control,encoder->mode());
	
}

void CarbonStream::activateMenuBar() {
	OSStatus err;
	err = SetMenuBarFromNib(nibRef, CFSTR("StreamMenu"));
	if(err != noErr) msg->error("Can't get MenuBar!!");
}

/****************************************************************************/
/* EVENT HANDLERS */
/****************************************************************************/

void qualityHandler (ControlRef theControl, ControlPartCode partCode) {
	CarbonStream *me;
	OSStatus err = GetControlProperty(theControl,CARBON_GUI_APP_SIGNATURE,
		QUALITY_PROPERTY,sizeof(CarbonStream *),NULL,&me);
	if(err==noErr) {
		me->updateQuality();
	}
}

static  OSErr ChooseFolder(EventRef event,CarbonStream *me)
{

	OSStatus		anErr;

	AEDescList	docList;				// list of docs passed in
	long		index, itemsInList;
	Boolean		wasAlreadyOpen;

	anErr = GetEventParameter( event, OPEN_DOCUMENT_DIALOG_PARAM, typeAEList,NULL,sizeof(AEDescList),NULL, &docList);

//	anErr = AECountItems( &docList, &itemsInList);			// how many files passed in
//	for (index = itemsInList; index > 0; index--)			// handle each file passed in
//	{	
		AEKeyword	keywd;
		DescType	returnedType;
		Size		actualSize;
		FSRef 		fileRef;
		FSCatalogInfo	theCatInfo;
		
//		anErr = AEGetNthPtr( &docList, index, typeFSRef, &keywd, &returnedType,
		anErr = AEGetNthPtr( &docList, 1, typeFSRef, &keywd, &returnedType,
						(Ptr)(&fileRef), sizeof( fileRef ), &actualSize );
		
		anErr = FSGetCatalogInfo( &fileRef, kFSCatInfoFinderInfo, &theCatInfo, NULL, NULL, NULL );
		
		if (anErr == noErr) {
			char path[2048]; /* XXX - hardcoded max filename size */
			FSRefMakePath (&fileRef,(UInt8 *)path,2048);
			me->recordStreamPath(path);
		}
//	}

	return anErr;
} // ChooseFloder

static OSStatus StreamEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    OSStatus err = noErr;
    CarbonStream *me = (CarbonStream *)userData;
	switch (GetEventKind (event))
    {
        case kEventWindowClose: 
			me->hide();
			return noErr;
            break;
		case kAEOpenDocuments:
			ChooseFolder(event,me);
			break;
		case kEventWindowActivated:
			me->activateMenuBar();
			break;
		default:
            break;
    }
    return CallNextEventHandler(nextHandler,event);
}

OSStatus StreamTabEventHandler(EventHandlerCallRef inCallRef, 
	EventRef inEvent, void* inUserData )
{
	CarbonStream        *me = (CarbonStream *)inUserData;
	if(me->updateStreamTab()) return(noErr);
	else return(eventNotHandledErr);
}

OSStatus ServerTabEventHandler(EventHandlerCallRef inCallRef,
	EventRef inEvent, void* inUserData)
{
	CarbonStream        *me = (CarbonStream *)inUserData;
	if(me->updateServerTab()) return(noErr);
	else return( eventNotHandledErr );
}

/****************************************************************************/
/* COMMAND HANDLERS */
/****************************************************************************/

static OSStatus streamCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    HICommand command; 
    OSStatus err = noErr;
	SInt16 val;
	bool k;
    CarbonStream *me = (CarbonStream *)userData;
	err = GetEventParameter (event, kEventParamDirectObject,
        typeHICommand, NULL, sizeof(HICommand), NULL, &command);
    if(err != noErr) me->msg->error("Can't get event parameter!!");
	switch (command.commandID)
    {
		case CHANGE_ENCTYPE_CMD:
			me->codecChange();
			break;
		case QUALITY_CMD:
			me->updateQuality();
			break;
		case BITRATE_CMD:
			me->updateBitrate();
			break;
		case FREQUENCY_CMD:
			me->updateFrequency();
			break;
		case MODE_CMD:
			me->updateMode();
			break;
		case BROWSE_OUTDIR_CMD:
			OpenFolderWindow(me->window);
			break;
		  case ADD_STREAM_CMD:
			me->addStream();
			break;
		case ADD_SERVER_CMD:
			me->addServer();
			break;
		case DELETE_STREAM_CMD:
			me->deleteStream();
			break;
		case DELETE_SERVER_CMD:
			me->deleteServer();
			break;
		case CONNECT_CMD:
			me->doConnect();
			break;
		default:
			err = eventNotHandledErr;
			break;
	}
	return err;
}