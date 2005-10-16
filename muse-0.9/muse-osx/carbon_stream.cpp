/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2005 xant <xant@dyne.org>
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
 */

#include "carbon_stream.h"
#include <jutils.h>
#include <sys/stat.h>

extern "C" OSStatus OpenFolderWindow(WindowRef parent);
void QualityHandler (ControlRef theControl, ControlPartCode partCode);

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

static const EventTypeSpec serverTextEvents[] = {
	{kEventClassControl, kEventControlHit},
	{kEventClassControl,kEventControlSetFocusPart}
};

const EventTypeSpec windowCommands[] = {
	{ kEventClassCommand, kEventCommandProcess }
};

static OSStatus StreamEventHandler (
	EventHandlerCallRef nextHandler, EventRef event, void *userData);
	
static OSStatus StreamCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);

OSStatus StreamTabEventHandler(EventHandlerCallRef inCallRef, 
	EventRef inEvent, void* inUserData );
	
OSStatus ServerTabEventHandler(EventHandlerCallRef inCallRef, 
	EventRef inEvent, void* inUserData );

OSStatus ServerTextEventHandler(EventHandlerCallRef inCallRef,
	EventRef inEvent, void* inUserData);
	
static OSStatus SavePresetCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);
	

#define kLeftArrow   0x1C
#define kRightArrow  0x1D
#define kUpArrow     0x1E
#define kDownArrow   0x1F
#define kBackspace   0x08
#define kDelete	     0x18
#define kCopy        0x63
#define kPaste       0x76
#define kCut         0x78

ControlKeyFilterResult ServerTextFilterProc (ControlRef theControl,
   SInt16 * keyCode, SInt16 * charCode,EventModifiers * modifiers);	

void  ServerTextValidator(ControlRef theControl);
	
#define CS_ALLOWED_BPS_NUM 8
#define CS_ALLOWED_FREQ_NUM 4
const int bps[CS_ALLOWED_BPS_NUM] = { 16,24,32,48,56,64,96,128 };
const int freq[CS_ALLOWED_FREQ_NUM] = { 11000,16000,22050,44100 };

/****************************************************************************/
/* CarbonStream class */
/****************************************************************************/

CarbonStream::CarbonStream(Stream_mixer *mix,WindowRef mainWin,IBNibRef nib) 
{
		parent=mainWin;
		jmix=mix;
		nibRef=nib;
		OSStatus err;
		
		_selectedStream = 0;
		_selectedServer = 0;
		
		memset(enc,0,sizeof(enc));
		memset(servers,0,sizeof(servers)); 
		msg = new CarbonMessage(nibRef);
		
		/* create the stream window using received nibref */
		err = CreateWindowFromNib(nibRef,CFSTR("StreamWindow"),&window);
		if(err != noErr)
			msg->error("Can't create the stream configuration window (%d)!!",err);
		
		/* now create the menu to use as new menubar */
		err=CreateMenuFromNib(nibRef,CFSTR("StreamMenu"),&streamMenu);
		if(err!=noErr) msg->error("Can't create streamMenu (%d)!!",err);
		
		/* Create a window group and put the stream window inside it ...
		 * this is done just to let Quartz handle window layering correctly */
		err=CreateWindowGroup(kWindowGroupAttrMoveTogether|kWindowGroupAttrLayerTogether|
			kWindowGroupAttrSharedActivation|kWindowGroupAttrHideOnCollapse,&streamGroup);
		err=SetWindowGroup(window,streamGroup);
		
		err = InstallEventHandler(GetWindowEventTarget(window),StreamEventHandler,STREAM_EVENTS,windowEvents,this,NULL);
		if(err != noErr)
			msg->error("Can't install event handler for Stream control (%d)!!",err);
			
		/* install the stream command handler */
		err = InstallWindowEventHandler (window, 
            NewEventHandlerUPP (StreamCommandHandler), 
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
		
		/* init all server textcontrols */ 
		initServerControls();
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
		SetControlAction(qualityControl,NewControlActionUPP(&QualityHandler));
		
		/* init xml presets */  /* XXX - DUPLICATE CODE ... ALSO playlist_manager ... should be generalized */
		struct stat st;
		char *home=NULL;
		char *repository=NULL;
		if(!(home = getenv("HOME"))) {
			error(_("no $HOME found"));
			repository=(char *)malloc(3);
			sprintf(repository,"./");
		}
		else {
			repository=(char *)malloc(strlen(home)+8);
			sprintf(repository,"%s/.muse/",home);
			if(stat(repository,&st)!=0) {
				if(mkdir(repository,S_IRWXU)!=0) {
					error("Can't create preferences directory %s !!",repository);
					/* TODO - should printout errno */
				}
			}
		}
		presets=new XmlProfile("stream_presets",repository);
		free(repository);
		updatePresetControls();
		
		/* init savePreset window */
		err=CreateWindowFromNib(nibRef, CFSTR("SavePresetWindow"), &savePresetWindow);
		if(err != noErr) msg->error("Can't create the savePreset window (%d)!!",err);
		err = InstallWindowEventHandler (savePresetWindow,NewEventHandlerUPP (SavePresetCommandHandler), 
			GetEventTypeCount(windowCommands), windowCommands, this, NULL);
		if(err != noErr) msg->error("Can't install savePreset commandHandler");
}

CarbonStream::~CarbonStream() {
	int i,n;
	ReleaseWindowGroup(streamGroup);
	for(i=0;i<MAX_STREAM_ENCODERS;i++) {
		if(enc[i]) {
			for(n=0;n<MAX_STREAM_SERVERS;n++) {
				if(servers[i][n]) {
					delete servers[i][n];
					servers[i][n] = NULL;
				}
			}
			delete enc[i];
			enc[i] = NULL;
		}
	}
	if(presets) delete presets;
	DisposeControlKeyFilterUPP(textFilterRoutine);
	DisposeControlEditTextValidationUPP(textValidationRoutine);
	DisposeWindow(savePresetWindow);
	DisposeMenu(streamMenu);
	DisposeWindow(window);
}
/*
CarbonStream::run() {
	if(IsControlVisible()) {
	}
}
*/

void CarbonStream::show() {
	RepositionWindow(window,parent,kWindowCenterOnMainScreen);
	ShowWindow(window);
	BringToFront(window);
	ActivateWindow(window,true);
	if(!IsControlVisible(streamTabControl)) addStream();
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

void CarbonStream::delTab(SInt32 controlID,int idx) {
	ControlRef *control;
	SELECT_TAB_CONTROL(controlID,control);
	SInt32 sNum=GetControl32BitMaximum(*control);
	if(sNum > 1) {
		SInt32 selected = GetControl32BitValue(*control);
	//	if(selected==idx) SetControl32BitValue(*control,selected>1?(selected-1):(selected+1));
		for(int i=idx;i<sNum;i++) {
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

/* Add a new stream encoder and update tabs */
int CarbonStream::addStream() {
	SInt32 sNum=GetControl32BitMaximum(streamTabControl);
	if(!IsControlVisible(streamTabControl)) { /* First stream */ 
		if(enc[0]) {
			delete enc[0];
		}
		enc[0] = new CarbonStreamEncoder(jmix,msg);
		if(!enc[0]) {
			/* XXX - errors here */
			return -1;
		}
		setTabValue(STREAM_TAB_CONTROL,1,1);
		ShowControl(streamTabControl);
	//	EnableControl(streamTabControl);
	
		updateStreamTab();
		return 0;
	}
	else if(addTab(STREAM_TAB_CONTROL)) { /* new tab must be created */
		int encIdx = getTabValue(STREAM_TAB_CONTROL,sNum+1)-1;
		if(enc[encIdx]) {
			delete enc[encIdx];
		}
		enc[encIdx]=new CarbonStreamEncoder(jmix,msg);
		if(!enc[encIdx]) {
			/* XXX - errors here */
			return -1;
		}
		updateStreamTab();
		return encIdx;
	}
	return -1;
}

void CarbonStream::deleteStream(int idx) {
	if(idx > MAX_STREAM_ENCODERS) {
		/* ERRORS HERE */
		return;
	} 
	int numServers=GetControl32BitMaximum(serverTabControl);
	if(enc[idx]) {
		for (int i=MAX_STREAM_SERVERS-1;i>=0;i--) {
			if(servers[idx][i]) {
				deleteServer(idx,i);
			}
		}
	//	if(IsControlVisible(serverTabControl)) {
	//		for(int i=numServers;i>0;i--) {
	//			deleteServer(idx,i);
	//		}
	//	}
		delete enc[idx];
		enc[idx]=NULL;
	}
}

void CarbonStream::deleteStream() {
	SInt32 selected=GetControl32BitValue(streamTabControl);
	int encIdx=getTabValue(STREAM_TAB_CONTROL,selected)-1;
	deleteStream(encIdx);
	delTab(STREAM_TAB_CONTROL,selected);
	if(_selectedStream==selected) _selectedStream=0;
	if(!updateStreamTab()) changeServerTab(); 
}

CarbonStreamEncoder *CarbonStream::getStream(int idx) {
	if(idx>=0) return enc[idx];
	return NULL;
}

void CarbonStream::deleteServer() {
	SInt32 selectedStream=GetControl32BitValue(streamTabControl);
	SInt32 selectedServer=GetControl32BitValue(serverTabControl);
	int encIdx=getTabValue(STREAM_TAB_CONTROL,selectedStream)-1;
	int serIdx=getTabValue(SERVER_TAB_CONTROL,selectedServer)-1;
	deleteServer(encIdx,serIdx);
	delTab(SERVER_TAB_CONTROL,selectedServer);
	if(IsControlVisible(serverTabControl)) {
		SInt32 newSel = GetControl32BitValue(serverTabControl);
		int newIdx=getTabValue(SERVER_TAB_CONTROL,newSel)-1;
		if(servers[encIdx][newIdx]) 
			updateServerInfo(servers[encIdx][newIdx]);
	}
}

void CarbonStream::deleteServer(int encIdx,int serIdx) {
	if(serIdx <0 || serIdx > MAX_STREAM_SERVERS || encIdx < 0 || encIdx > MAX_STREAM_ENCODERS) return;
	if(servers[encIdx][serIdx]) {
		delete servers[encIdx][serIdx];
		servers[encIdx][serIdx]=NULL;
	}
}

int CarbonStream::addServer() {
	if(IsControlVisible(streamTabControl)) {
		int selectedStream = getTabValue(STREAM_TAB_CONTROL,GetControl32BitValue(streamTabControl))-1;
		return addServer(selectedStream);
	}
	return -1;
}

int CarbonStream::addServer(int strIdx) {
	SInt32 sNum=GetControl32BitMaximum(serverTabControl);
	CarbonStreamEncoder *sEnc = enc[strIdx];
	if(!sEnc) {
		msg->warning("Can't get encoder object for the selected stream !!");
		return -1;
	}
	if(!IsControlVisible(serverTabControl)) { /* First server */ 
		servers[strIdx][0]=new CarbonStreamServer(sEnc);
		setTabValue(SERVER_TAB_CONTROL,1,1);
		ShowControl(serverTabControl);
	//	EnableControl(serverTabControl);
		
		updateServerTab();
		return 0;
	}
	else { /* new tab must be created */
		if(addTab(SERVER_TAB_CONTROL)) {
			int serverIndex=getTabValue(SERVER_TAB_CONTROL,sNum+1)-1;
			if(servers[strIdx][serverIndex]) {
				/* XXX - error messages here */
				delete servers[strIdx][serverIndex];
			}
			servers[strIdx][serverIndex]=new CarbonStreamServer(sEnc);
			if(!updateServerTab())
				updateServerInfo(servers[strIdx][serverIndex]);
			return serverIndex;
		}
	}
	return -1;
}

CarbonStreamServer *CarbonStream::getServer(int strIdx,int srvIdx) {
	if(strIdx>=0&&srvIdx>=0) return servers[strIdx][srvIdx];
	return NULL;
}

bool CarbonStream::updateStreamTab() {
	SInt32 val = GetControl32BitValue(streamTabControl);
	if(_selectedStream==val) return false; /* no changes ... */
	_selectedStream=val;
	int idx = getTabValue(STREAM_TAB_CONTROL,_selectedStream)-1;
	if(idx < 0) return false;
	if(enc[idx]) saveStreamInfo(enc[idx]);
	if(IsControlVisible(streamTabControl)) {
		changeServerTab();
		updateStreamInfo(selectedStream());
	}
	return true;
}

bool CarbonStream::updateServerTab() {
	SInt32 val = GetControl32BitValue(serverTabControl);
	SInt32 max = GetControl32BitMaximum(serverTabControl);
	int streamIndex=getTabValue(STREAM_TAB_CONTROL,_selectedStream)-1;
	if(_selectedServer>max) _selectedServer=max;
	else if(_selectedServer==val) {
		saveServerInfo(selectedServer());
		updateServerInfo(selectedServer());
		return false;
	}
	if(_selectedServer && IsControlVisible(serverTabControl)) { 
		int serverIndex=getTabValue(SERVER_TAB_CONTROL,_selectedServer)-1;
		saveServerInfo(servers[streamIndex][serverIndex]);
	}
	_selectedServer=val;
	int newServerIndex=getTabValue(SERVER_TAB_CONTROL,_selectedServer)-1;
	if(streamIndex >= 0 && newServerIndex >= 0 && servers[streamIndex][newServerIndex])
		updateServerInfo(servers[streamIndex][newServerIndex]);
	return true;
}

bool CarbonStream::changeServerTab() {
	SInt32 val = GetControl32BitValue(streamTabControl);
	int newStreamIndex=getTabValue(STREAM_TAB_CONTROL,val)-1;
	int oldServerIndex=getTabValue(SERVER_TAB_CONTROL,_selectedServer)-1;
	int sNum=0;
	_selectedServer=1;
	if(_selectedStream && IsControlVisible(serverTabControl)) {
		int oldStreamIndex=getTabValue(STREAM_TAB_CONTROL,_selectedStream)-1;
		if(oldStreamIndex >= 0 && oldServerIndex >= 0)
			if(servers[oldStreamIndex][oldServerIndex])
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

void CarbonStream::saveStreamInfo(CarbonStreamEncoder *encoder) {
	ControlRef control;
	ControlID cid={CARBON_GUI_APP_SIGNATURE,0};
	OSStatus err;
	
	/* record filename text control */
	cid.id=RECORD_STREAM_FILENAME;
	err=GetControlByID(window,&cid,&control);
	if(err!=noErr) msg->error("Can't get recordFilename control (%d)!!",err);
	Size nameSize;
	err=GetControlDataSize(control,0,kControlEditTextTextTag,&nameSize);
	if(err!=noErr) msg->error("Can't get text size for saveName (%d)!!\n",err);
	char *outFilename=(char *)malloc(nameSize+1);
	memset(outFilename,0,nameSize+1);
	err=GetControlData(control,0,kControlEditTextTextTag,nameSize,outFilename,NULL);
	if(err==noErr) {
		encoder->saveFile(outFilename);
	}
	free(outFilename);
}

#define GET_SERVER_CONTROL(__win,__cid,__id,__c) \
	__cid.id=__id;\
	if(GetControlByID(__win,&__cid,&__c)!=noErr) \
		msg->error("Can't get __c server control!!");\
	if(SetControlProperty(__c,CARBON_GUI_APP_SIGNATURE,SERVER_TEXT_PROPERTY,\
		sizeof(CarbonStream *),&self)!=noErr) \
		msg->error("Can't attach CarbonChannel object to a serverText control!!");

#define INIT_SERVER_TEXT_CONTROL(__win,__cid,__id,__c,__r1,__r2) \
	GET_SERVER_CONTROL(__win,__cid,__id,__c);\
	SetControlData(__c,kControlNoPart,kControlEditTextKeyFilterTag,sizeof(__r1),(Ptr) &__r1);\
	SetControlData(__c,kControlNoPart,kControlEditTextValidationProcTag,sizeof(__r2),(Ptr) &__r2);\
	if(InstallControlEventHandler (__c,NewEventHandlerUPP(ServerTextEventHandler), \
		GetEventTypeCount(serverTextEvents),serverTextEvents,this, NULL)!=noErr)\
		msg->error("Can't install eventHandler for a serverText control!!");

void CarbonStream::initServerControls() {
	ControlID cid = {CARBON_GUI_APP_SIGNATURE,0};
	textFilterRoutine=NewControlKeyFilterUPP(&ServerTextFilterProc);
	textValidationRoutine=NewControlEditTextValidationUPP(ServerTextValidator); 
	CarbonStream *self=this;
	OSStatus err;

	/* Here we also initialize internal ControlRefs for all those controls */
	INIT_SERVER_TEXT_CONTROL(window,cid,HOST_CONTROL,serverHost,textFilterRoutine,textValidationRoutine);
	INIT_SERVER_TEXT_CONTROL(window,cid,PORT_CONTROL,serverPort,textFilterRoutine,textValidationRoutine);
	INIT_SERVER_TEXT_CONTROL(window,cid,MNT_CONTROL,serverMount,textFilterRoutine,textValidationRoutine);
	INIT_SERVER_TEXT_CONTROL(window,cid,NAME_CONTROL,serverName,textFilterRoutine,textValidationRoutine);
	INIT_SERVER_TEXT_CONTROL(window,cid,URL_CONTROL,serverUrl,textFilterRoutine,textValidationRoutine);
	INIT_SERVER_TEXT_CONTROL(window,cid,DESCRIPTION_CONTROL,serverDescr,textFilterRoutine,textValidationRoutine);
	INIT_SERVER_TEXT_CONTROL(window,cid,USERNAME_CONTROL,serverUser,textFilterRoutine,textValidationRoutine);
	INIT_SERVER_TEXT_CONTROL(window,cid,PASSWORD_CONTROL,serverPass,textFilterRoutine,textValidationRoutine);
	
	cid.id=CONNECT_BUTTON;
	err=GetControlByID(window,&cid,&serverConnectButton);
	if(err!=noErr) msg->error("Can't get serverConnectButton control (%d)!!",err);
//	err=SetControlProperty(serverConnectButton,CARBON_GUI_APP_SIGNATURE,SERVER_TEXT_PROPERTY,
//		sizeof(CarbonStream *),&self);
//	if(err!=noErr) msg->error("Can't attach CarbonStream pointer to serverConnectButtonControl (%d)!!",err);
//	InstallControlEventHandler(serverConnectButton,NewEventHandlerUPP(ServerTextEventHandler),
//		GetEventTypeCount(serverTextEvents),serverTextEvents,this, NULL);
}

#define SAVE_SERVER_INFO(__c) \
	{\
		err=GetControlData(__c,0,kControlEditTextTextTag,SERVER_STRING_BUFFER_LEN-1,buffer,NULL);\
		if(err!=noErr) msg->error("Can't get %s from text control (%d)!!","__name",err);\
	}
	
#define SAVE_SERVER_TEXT_INFO(__c,__func) \
	{\
		SAVE_SERVER_INFO(__c)\
		server->__func(buffer);\
		memset(buffer,0,sizeof(buffer));\
	}
	
#define SAVE_SERVER_INT_INFO(__c,__func) \
	{\
		SAVE_SERVER_INFO(__c)\
		intBuffer=0;\
		if(sscanf(buffer,"%d",&intBuffer) == 1) \
		{\
			server->__func(intBuffer);\
			intBuffer=0;\
			memset(buffer,0,sizeof(buffer));\
		}\
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
	SAVE_SERVER_INT_INFO(serverPort,port);
	/* login type */
	//SAVE_SERVER_INT_INFO(server	
	/* host */
	SAVE_SERVER_TEXT_INFO(serverHost,host);
	/* mnt */
	SAVE_SERVER_TEXT_INFO(serverMount,mount);
	/* name */
	SAVE_SERVER_TEXT_INFO(serverName,name);
	/* url */
	//SAVE_SERVER_TEXT_INFO(serverUrl,url);
	/*description */
	SAVE_SERVER_TEXT_INFO(serverDescr,description);
	/* username */
	SAVE_SERVER_TEXT_INFO(serverUser,username);
	/* password */
	SAVE_SERVER_TEXT_INFO(serverPass,password);
}

#define UPDATE_SERVER_TEXT_INFO(__c,__func) \
	{\
		buffer = server->__func();\
		len=buffer?strlen(buffer):0;\
		err=SetControlData(__c,0,(__c==serverPass)?kControlEditTextPasswordTag:\
			kControlEditTextTextTag,len,buffer);\
		if(err!=noErr) msg->warning("Can't set __name for text control (%d)!!",err);\
		buffer=NULL;\
	}

#define UPDATE_SERVER_INT_INFO(__c,__func) \
	{\
		intBuf = server->__func();\
		if(intBuf) sprintf(intBufStr,"%d",intBuf);\
		else *intBufStr=0;\
		len=strlen(intBufStr);\
		err=SetControlData(__c,0,kControlEditTextTextTag,len,intBufStr);\
		if(err!=noErr) msg->warning("Can't set __name from text control (%d)!!",err);\
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
	
	if(!server) return;
	
	/* host */
	UPDATE_SERVER_TEXT_INFO(serverHost,host);
	/* mnt */
	UPDATE_SERVER_TEXT_INFO(serverMount,mount);
	/* name */
	UPDATE_SERVER_TEXT_INFO(serverName,name);
	/* url */
	UPDATE_SERVER_TEXT_INFO(serverUrl,url);
	/*description */
	UPDATE_SERVER_TEXT_INFO(serverDescr,description);
	/* username */
	UPDATE_SERVER_TEXT_INFO(serverUser,username);
	/* password */
	UPDATE_SERVER_TEXT_INFO(serverPass,password);
	
	/*port */
	UPDATE_SERVER_INT_INFO(serverPort,port);
	
	/* connect button */
	if(server->isConnected()) {
		SetControlTitleWithCFString(serverConnectButton,CFSTR("Disconnect"));
	}
	else {
		SetControlTitleWithCFString(serverConnectButton,CFSTR("Connect"));
	}
}

/*
 * isConnected?disconnect:connect 
 * - do the right action when the user presses the connect/disconnect button 
 */
bool CarbonStream::doConnect() {
	CarbonStreamServer *server=selectedServer();
	saveServerInfo(server);
	bool res=false;
	if(server->isConnected()) res=disconnectServer(server);
	else res=connectServer(server);
	updateServerInfo(server);
	return res;
}

bool CarbonStream::disconnectServer(CarbonStreamServer *server) {
	if(server) return server->disconnect();
}

/* try connection to the shoutcast server */
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

bool CarbonStream::updateQuality() {
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
	int cur=enc->quality();
	if(quality!=cur) {
		enc->quality(quality);
		if(!enc->update()) {
			/* TODO - WARNING MESSAGES ?? */
			enc->quality(cur); /* restore old value */
		}
		updateStreamInfo(enc);
		return true;
	}
	return false;
}

bool CarbonStream::updateBitrate() {
	CarbonStreamEncoder *enc=selectedStream();
	ControlRef bpsControl;
	const ControlID bpsID = { CARBON_GUI_APP_SIGNATURE,BITRATE_CONTROL };
	OSStatus err=GetControlByID(window,&bpsID,&bpsControl);
	if(err!=noErr) msg->error("Can't get bps control (%d)!!",err);
	SInt32 val = GetControlValue(bpsControl);
	int cur = enc->bitrate();
	enc->bitrate(bps[val-1]);
	if(!enc->update()) {
		/* TODO - WARNING MESSAGES ?? */
		enc->bitrate(cur); /* restore old value */
		updateStreamInfo(enc);
	}
	return true;
}

bool CarbonStream::updateFrequency() {
	CarbonStreamEncoder *enc=selectedStream();
	ControlRef freqControl;
	const ControlID freqID = { CARBON_GUI_APP_SIGNATURE,FREQUENCY_CONTROL };
	OSStatus err=GetControlByID(window,&freqID,&freqControl);
	if(err!=noErr) msg->error("Can't get frequency control (%d)!!",err);
	SInt32 val = GetControlValue(freqControl);
	int cur = enc->frequency();
	enc->frequency(freq[val-1]);
	if(!enc->update()) {
		/* TODO - WARNING MESSAGES ?? */
		enc->frequency(cur); /* restore old value */
		updateStreamInfo(enc);
	}
	return true;
}

bool CarbonStream::updateMode() {
	CarbonStreamEncoder *enc=selectedStream();
	ControlRef modeControl;
	const ControlID modeID = { CARBON_GUI_APP_SIGNATURE,MODE_CONTROL };
	OSStatus err=GetControlByID(window,&modeID,&modeControl);
	if(err!=noErr) msg->error("Can't get mode control (%d)!!",err);
	SInt32 val = GetControlValue(modeControl);
	int cur=enc->mode();
	enc->mode(val);
	if(!enc->update()) {
		/* TODO - WARNING MESSAGES ?? */
		enc->mode(cur);
		updateStreamInfo(enc);
	}
	return true;
}

bool CarbonStream::updateFilteringMode() {
	ControlRef control;
	ControlID cid={CARBON_GUI_APP_SIGNATURE,FREQUENCY_FILTER_CONTROL};
	OSStatus err=GetControlByID(window,&cid,&control);
	if(err!=noErr) msg->error("Can't get freqfiltering control (%d)!!",err);
	SInt16 sel=GetControlValue(control);
	CarbonStreamEncoder *enc=selectedStream();
	enc->filterMode(sel-1);
	//enc->update();
	updateStreamInfo(enc);
	return true;
}

void CarbonStream::recordStreamPath(char *path) {
	SInt32 selectedStreamIndex=GetControl32BitValue(streamTabControl);
	CarbonStreamEncoder *encoder=selectedStream();
	char *outFilename = (char *)malloc(strlen(path)+16); /* XXX - HC LEN */
	sprintf(outFilename,"%s/Stream_%d.%s",path,selectedStreamIndex,encoder->type()==OGG?"ogg":"mp3");
	if(encoder) encoder->saveFile(outFilename); /* XXX - no need of check on encoder */
	updateStreamInfo(encoder);
	free(outFilename);
}

void CarbonStream::recordStream() {
	CarbonStreamEncoder *encoder=selectedStream();
	if(encoder) {
		saveStreamInfo(encoder);
		if(encoder->isSaving()) {
			if(!encoder->stopSaving()) 
				msg->warning("Can't stop dumping on file");
		}
		else {
			if(!encoder->startSaving()) 
				msg->warning("Can't start dumping on file");
		}
		updateStreamInfo(encoder);
	}
}

void CarbonStream::codecChange() {
	CarbonStreamEncoder *enc=selectedStream();
	ControlRef typeControl;
	const ControlID typeID = { CARBON_GUI_APP_SIGNATURE,ENCODER_SELECT_CONTROL };
	OSStatus err=GetControlByID(window,&typeID,&typeControl);
	if(err!=noErr) msg->error("Can't get type control (%d)!!",err);
	SInt32 val = GetControlValue(typeControl);
	enc->type((val==1)?OGG:MP3);
	enc->update();
	updateStreamInfo(enc);
}

void CarbonStream::updatePresetControls() {
	MenuRef loadMenu,deleteMenu,saveMenu;
	ControlRef loadButton,deleteButton,saveButton;
	ControlID loadButtonId = {CARBON_GUI_APP_SIGNATURE,LOAD_STREAM_PRESET_BUT};
	ControlID deleteButtonId = {CARBON_GUI_APP_SIGNATURE,DELETE_STREAM_PRESET_BUT};
	ControlID saveButtonId = {CARBON_GUI_APP_SIGNATURE,SAVE_STREAM_PRESET_BUT};

	/* LOAD PRESET BBUTTON */
	OSStatus err=GetControlByID(window,&loadButtonId,&loadButton);
	if(err!=noErr) msg->error("Can't get controlref for loadPreset button (%d)!!",err);
	err=GetBevelButtonMenuHandle(loadButton,&loadMenu);
	if(err!=noErr) msg->error("Can't get menuref for the loadPreset button (%d)!!",err);
	UInt16 nItems=CountMenuItems(loadMenu);
	err=DeleteMenuItems(loadMenu,1,nItems);
	err=SetMenuFont(loadMenu,0,9);
	/* DELETE PRESET BUTTON */
	err=GetControlByID(window,&deleteButtonId,&deleteButton);
	if(err!=noErr) msg->error("Can't get controlref for deletePreset button (%d)!!",err);
	err=GetBevelButtonMenuHandle(deleteButton,&deleteMenu);
	if(err!=noErr) msg->error("Can't get menuref for the deletePreset button (%d)!!",err);
	nItems=CountMenuItems(deleteMenu);
	err=DeleteMenuItems(deleteMenu,1,nItems);
	err=SetMenuFont(deleteMenu,0,9);

	int npr = presets->numBranches();
	for(int i=1;i<=npr;i++) {
		XmlTag *profile=presets->getBranch(i);
		if(profile && strcmp(profile->name(),"profile")==0) {
			char *name = profile->value();
			if(name) {
				MenuItemIndex newIdx;
				CFStringRef text=CFStringCreateWithCString(NULL,name,kCFStringEncodingMacRoman );
				err=AppendMenuItemTextWithCFString(loadMenu,text,0,LOAD_STREAM_PRESET_CMD,&newIdx);
				err=AppendMenuItemTextWithCFString(deleteMenu,text,0,DELETE_STREAM_PRESET_CMD,&newIdx);
				CFRelease(text);
			}
			else {
				warning("Can't obtain name fro stream_preset entry at index %d",i);
				presets->removeBranch(i);
				presets->update();
				i--;
				npr--;
			}
		}
		else {
			warning("Bad xml element at index %d while looking from stream presets, found %s",i,profile->name()); 
			presets->removeBranch(i);
			presets->update();
			i--;
			npr--;
		}
	}

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
	MenuRef chMenu=GetControlPopupMenuHandle(control);
/* XXX - THE FOLLOWING CODE IS NO MORE NEEDED */
//	if(encoder->type()==OGG) { /* XXX - DISABLED BECAUSE MUSE CRASH */
//		if(encoder->bitrate()>=48 && encoder->frequency()>=22050) {
//			EnableMenuItem(chMenu,2);
//		}
//		else {
//			DisableMenuItem(chMenu,2);
//		}
//	}
//	else {
//		EnableMenuItem(chMenu,2);
//	}
	SetControlValue(control,encoder->mode());
	/* frequency filtering */
	ControlRef hpControl,lpControl,applyButton;
	cid.id=FREQUENCY_FILTER_CONTROL;
	err=GetControlByID(window,&cid,&control);
	if(err!=noErr) msg->error("Can't get freqfiltering control (%d)!!",err);
	cid.id=LOWPASS_CONTROL;
	err=GetControlByID(window,&cid,&lpControl);
	if(err!=noErr) msg->error("Can't get lowPass control (%d)!!",err);
	cid.id=HIGHPASS_CONTROL;
	err=GetControlByID(window,&cid,&hpControl);
	if(err!=noErr) msg->error("Can't get highPass control (%d)!!",err);
	cid.id=FREQ_FILTER_APPLY;
	err=GetControlByID(window,&cid,&applyButton);
	if(err!=noErr) msg->error("Can't get applyButton control (%d)!!",err);
	if(encoder->filterMode()) {
		SetControlValue(control,2);
		EnableControl(lpControl);
		EnableControl(hpControl);
		EnableControl(applyButton);
	}
	else {
		SetControlValue(control,1);
		DisableControl(lpControl);
		DisableControl(hpControl);
		DisableControl(applyButton);

	}
	/* record fileName */
	cid.id=RECORD_STREAM_FILENAME;
	err=GetControlByID(window,&cid,&control);
	if(err!=noErr) msg->error("Can't get recordFilename control (%d)!!",err);
	char *outFilename=encoder->saveFile();
	if(outFilename) err=SetControlData(control,0,kControlEditTextTextTag,strlen(outFilename),outFilename);
	else err=SetControlData(control,0,kControlEditTextTextTag,0,NULL);
	if(err!=noErr) msg->warning("Can't update recordFilename text control (%d)!!",err);
	
	/* record button */
	cid.id=RECORD_STREAM_BUT;
	err=GetControlByID(window,&cid,&control);
	if(err!=noErr) msg->error("Can't get recordButton control (%d)!!",err);
	if(encoder->isSaving()) {
		SetControlTitleWithCFString(control,CFSTR("Stop recording"));
	}
	else {
		SetControlTitleWithCFString(control,CFSTR("Record now!"));
	}
}

void CarbonStream::activateMenuBar() {
	OSStatus err;
	err = SetRootMenu(streamMenu);
	if(err != noErr) msg->error("Can't get MenuBar!!");
}

bool CarbonStream::loadPreset(int idx) {
	XmlTag *profile = presets->getBranch(idx);
	int val,i,n,k;
	if(profile) {
		/* remove current streams */
		if(IsControlVisible(streamTabControl)) {
			SInt32 nStreams = GetControl32BitMaximum(streamTabControl);
			for(i=nStreams;i>0;i--) {
				deleteStream();
			}
		}
		for(i=1;i<=profile->numChildren();i++) { /* cycle on streams */
			XmlTag *entry=profile->getChild(i);
			if(entry) {
				if(strcmp(entry->name(),"stream")==0) { /* stream entry */
					int strIdx=addStream();
					CarbonStreamEncoder *encoder=getStream(strIdx);
					for(n=1;n<=entry->numChildren();n++) { /* cycle on stream cfg options */
						XmlTag *cfg=entry->getChild(n);
						if(cfg) {
							char *cfgName = cfg->name();
							if(strcmp(cfgName,"encoder")==0) {
								if(sscanf(cfg->value(),"%d",&val)==1) {
									if(val==0) encoder->type(OGG); /* XXX - HC relation beetwen 0/1 and OGG/MP3 */
									else if(val==1) encoder->type(MP3);
									encoder->update(); /* extra update here to change codec before creating servers */
								}
							}
							else if(strcmp(cfgName,"quality")==0) {
								if(sscanf(cfg->value(),"%d",&val)==1) {
									encoder->quality(val);
								}
							}
							else if(strcmp(cfgName,"mode")==0) {
								if(sscanf(cfg->value(),"%d",&val)==1) {
									encoder->mode(val);
								}
							}
							else if(strcmp(cfgName,"bitrate")==0) {
								if(sscanf(cfg->value(),"%d",&val)==1) {
									encoder->bitrate(val);
								}
							}
							else if(strcmp(cfgName,"frequency")==0) {
								if(sscanf(cfg->value(),"%d",&val)==1) {
									encoder->frequency(val);
								}
							}
							else if(strcmp(cfgName,"ffiltering")==0) {
								/* UNIMPLEMENTED */
							}
							else if(strcmp(cfgName,"lowpass")==0) {
						//		if(sscanf(cfg->value(),"%d",&val)==1) {
									encoder->lowpass(0); /* UNIMPLEMENTED */
							//	}
							}
							else if(strcmp(cfgName,"highpass")==0) {
							//	if(sscanf(cfg->value(),"%d",&val)==1) {
									encoder->highpass(0); /* UNIMPLEMENTED */
							//	}
							}
							else if(strcmp(cfgName,"filename")==0) {
								encoder->saveFile(cfg->value());
							}
							else if(strcmp(cfgName,"server")==0) { /* server entry */
								int srvIdx=addServer(strIdx);
								if(srvIdx>=0) {
									CarbonStreamServer *server=getServer(strIdx,srvIdx);
									for(k=1;k<=cfg->numChildren();k++) { /* cycle on server options */
										XmlTag *serverCfg=cfg->getChild(k);
										if(serverCfg) {
											char *opt=serverCfg->name();
											if(strcmp(opt,"host")==0) {
												if(serverCfg->value())
													server->host(serverCfg->value());
											}
											else if(strcmp(opt,"port")==0) {
												if(serverCfg->value()) {
												if(sscanf(serverCfg->value(),"%d",&val)==1) {
													server->port(val);
												}
												}
											}
											else if(strcmp(opt,"mount")==0) {
												if(serverCfg->value())
													server->mount(serverCfg->value());
											}
											else if(strcmp(opt,"name")==0) {
												if(serverCfg->value())
													server->name(serverCfg->value());
											}
											else if(strcmp(opt,"url")==0) {
												if(serverCfg->value())
													server->url(serverCfg->value());
											}
											else if(strcmp(opt,"description")==0) {
												if(serverCfg->value())
													server->description(serverCfg->value());
											}
											else if(strcmp(opt,"ltype")==0) {
												if(serverCfg->value()) {
												if(sscanf(serverCfg->value(),"%d",&val)==1) {
													server->loginType(val);
												}
												}
											}
											else if(strcmp(opt,"username")==0) {
												if(serverCfg->value()) 
												server->username(serverCfg->value());
											}
											else if(strcmp(opt,"password")==0) { /* XXX - mmm...clean password in the cfg file */
												if(serverCfg->value())
												server->password(serverCfg->value());
											}
										} /* end of server entry */
										updateServerInfo(server); /* make the new changes on server visible in the gui */
									} /* end of cycle on server options */
								} 
							} /* end of strean entry */
							else {
								
							}
						}
					}
					encoder->update();
					updateStreamInfo(encoder); /* make the new changes on encoder visible in the gui */
				} /* end of stream entry */
			} /* end of cycle on streams */
		}
	}
	return false;
}

bool CarbonStream::deletePreset(int idx) {
	if(presets->removeBranch(idx)) {
		updatePresetControls();
		if(presets->update()) 
			return true;
	}
	return false;
}

bool CarbonStream::savePreset(char *name) {
	int i,n;
	XmlTag *newProfile = new XmlTag("profile",name,NULL);
	char int2str[256];
	for(i=0;i<MAX_STREAM_ENCODERS;i++) {
		if(enc[i]) {
			XmlTag *newStream = new XmlTag("stream",newProfile);
			if(enc[i]->type() != DEFAULT_ENCODER) {
				newStream->addChild("encoder","1");
			}
			if(enc[i]->quality() != DEFAULT_QUALITY) {
				sprintf(int2str,"%d",enc[i]->quality());
				newStream->addChild("quality",int2str);
			}
			if(enc[i]->bitrate()!=DEFAULT_BITRATE) {
				sprintf(int2str,"%d",enc[i]->bitrate());
				newStream->addChild("bitrate",int2str);
			}
			if(enc[i]->mode() != DEFAULT_MODE) {
				sprintf(int2str,"%d",enc[i]->mode());
				newStream->addChild("mode",int2str);
			}
			if(enc[i]->frequency() != DEFAULT_FREQUENCY) {
				sprintf(int2str,"%d",enc[i]->frequency());
				newStream->addChild("frequency",int2str);
			}
			for(n=0;n<MAX_STREAM_SERVERS;n++) {
				if(servers[i][n]) {
					XmlTag *newServer=new XmlTag("server",newStream);
					sprintf(int2str,"%d",servers[i][n]->port());
					if(servers[i][n]->port()) newServer->addChild("port",int2str);
					
					if(servers[i][n]->host()) newServer->addChild("host",servers[i][n]->host());
					if(servers[i][n]->mount()) newServer->addChild("mount",servers[i][n]->mount());
					if(servers[i][n]->name()) newServer->addChild("name",servers[i][n]->name());
					if(servers[i][n]->url()) newServer->addChild("url",servers[i][n]->url());
					if(servers[i][n]->description()) newServer->addChild("description",servers[i][n]->description());
					if(servers[i][n]->username()) newServer->addChild("username",servers[i][n]->username());
					if(servers[i][n]->password()) newServer->addChild("password",servers[i][n]->password());
					newStream->addChild(newServer);
				}
			}
			newProfile->addChild(newStream);
		}
	}
	if(presets->addRootElement(newProfile)) {
		if(presets->update()) {
			updatePresetControls();
			return true;
		}
	}
	return false;
}

void CarbonStream::savePresetDialog() {
if(!IsWindowVisible(savePresetWindow)) {
		ShowSheetWindow(savePresetWindow,window);
	}
	else {
		BringToFront(savePresetWindow);
	}
	if(!HIViewSubtreeContainsFocus(HIViewGetRoot(savePresetWindow)))
		HIViewAdvanceFocus(HIViewGetRoot(savePresetWindow),0); /* set focus to the url input text box */
}

void CarbonStream::confirmSavePreset() {
	char *name=NULL;
	Size nameSize;
	ControlRef saveText;
	const ControlID saveTextID = { CARBON_GUI_APP_SIGNATURE, SAVE_PRESET_TEXT_CONTROL };
	OSStatus err;
	if(IsWindowVisible(savePresetWindow)) {
		err=GetControlByID(savePresetWindow,&saveTextID,&saveText);
		if(err!=noErr) msg->warning("Can't get text control from the savePreset dialog (%d)!!",err);
		err=GetControlDataSize(saveText,0,kControlEditTextTextTag,&nameSize);
		if(err!=noErr) msg->error("Can't get text size for saveName (%d)!!\n",err);
		name=(char *)malloc(nameSize+1);
		memset(name,0,nameSize+1);
		err=GetControlData(saveText,0,kControlEditTextTextTag,nameSize,name,NULL);
		if(err!=noErr) msg->error("Can't get text for saveName (%d)!!\n",err);
		SetControlData(saveText,0,kControlEditTextTextTag,0,NULL);
		if(strlen(name)>0) {
			HideSheetWindow(savePresetWindow);
			savePreset(name);
		}
		else {
			msg->warning("preset name can't be blank!!");
		}
		free(name);
	}
}

void CarbonStream::cancelSavePreset() {
	ControlRef saveText;
	const ControlID saveTextID = { CARBON_GUI_APP_SIGNATURE, SAVE_PRESET_TEXT_CONTROL };
	OSStatus err=GetControlByID(savePresetWindow,&saveTextID,&saveText);
	if(err!=noErr) msg->warning("Can't get text control from the savePreset dialog (%d)!!",err);
	SetControlData(saveText,0,kControlEditTextTextTag,0,NULL);
	HideSheetWindow(savePresetWindow);
}

/****************************************************************************/
/* EVENT HANDLERS */
/****************************************************************************/

void QualityHandler (ControlRef theControl, ControlPartCode partCode) {
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
	me->updateStreamTab();
	return CallNextEventHandler(inCallRef,inEvent); /* propagate event */
}

OSStatus ServerTabEventHandler(EventHandlerCallRef inCallRef,
	EventRef inEvent, void* inUserData)
{
	CarbonStream        *me = (CarbonStream *)inUserData;
	me->updateServerTab();
	return CallNextEventHandler(inCallRef,inEvent); /* propagate event */
}

OSStatus ServerTextEventHandler(EventHandlerCallRef inCallRef,
	EventRef inEvent, void* inUserData)
{
	CarbonStream *me=(CarbonStream *)inUserData;
	if(GetEventKind(inEvent)==kEventControlHit) return noErr; /* don't propagate clicks to the tab handler */
	me->updateServerTab();
/*	
	ControlRef control = (ControlRef) inUserData;
	ControlID cid;
	OSStatus err = GetControlID(control,&cid);
	if(!IsControlActive(control)) { 
		switch(cid.id) {
			case PORT_CONTROL:
				break;
			case URL_CONTROL:
			case MNT_CONTROL:
			case HOST_CONTROL:
			case NAME_CONTROL:
			case DESCRIPTION_CONTROL:
			case USERNAME_CONTROL:
			case PASSWORD_CONTROL:
				break;
		}
	}
*/
	return CallNextEventHandler(inCallRef,inEvent);
}

/****************************************************************************/
/* COMMAND HANDLERS */
/****************************************************************************/

static OSStatus StreamCommandHandler (
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
		case LOAD_STREAM_PRESET_CMD:
			me->loadPreset(command.menu.menuItemIndex);
			break;
		case SAVE_STREAM_PRESET_CMD:
			me->updateServerTab();
			me->savePresetDialog();
			break;
		case DELETE_STREAM_PRESET_CMD:
			me->deletePreset(command.menu.menuItemIndex);
			break;
		case FREQ_FILTERING:
			me->updateFilteringMode();
			break;
		case APPLY_FILTER_CMD:
			me->msg->warning("Manual frequency filtering is still unimplemented!!");
			break;
		case RECORD_STREAM_CMD:
			me->recordStream();
			break;
		default:
			err = eventNotHandledErr;
			break;
	}
	return err;
}

static OSStatus SavePresetCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    HICommand command; 
    OSStatus err = noErr;
	SInt16 val;
    CarbonStream *me = (CarbonStream *)userData;
	err = GetEventParameter (event, kEventParamDirectObject,
        typeHICommand, NULL, sizeof(HICommand), NULL, &command);
    if(err != noErr) me->msg->error("Can't get event parameter!!");
	switch (command.commandID)
    {
		case SAVE_PRESET_CONFIRM_CMD:
			me->confirmSavePreset();
			break;
		case CANCEL_CMD:
			me->cancelSavePreset();
			break;
		default:
			err = eventNotHandledErr;
	}
	return err;
}

/****************************************************************************/
/* TEXT CALLBACKS */
/****************************************************************************/

/* filter keystrokes on all server text controls */

ControlKeyFilterResult ServerTextFilterProc (ControlRef theControl,
   SInt16 * keyCode, SInt16 * charCode,EventModifiers * modifiers) 
{
	ControlKeyFilterResult res=kControlKeyFilterPassKey;
	ControlID cid;
	OSStatus err=GetControlID(theControl,&cid);
	/* allow arrow,backspace and delete keystrokes and all copy/cut/paste commands */
	if(*charCode != kLeftArrow && *charCode != kRightArrow && *charCode != kUpArrow
		&& *charCode != kDownArrow && *charCode != kBackspace && *charCode != kDelete &&
		*charCode != kCopy && *charCode != kCut && *charCode != kPaste)
	{
		switch(cid.id) {
			
			case PORT_CONTROL:
				if(*charCode < '0' || *charCode > '9')
					res=kControlKeyFilterBlockKey;
				break;
			case URL_CONTROL:
					res=kControlKeyFilterBlockKey; /* don't let user modify the url field */
				break;
			case MNT_CONTROL:
				if(*charCode == '/') res=kControlKeyFilterBlockKey;
				break;
			case HOST_CONTROL:
			case NAME_CONTROL:
			case DESCRIPTION_CONTROL:
			case USERNAME_CONTROL:
			case PASSWORD_CONTROL:
				break;
		}
	}
	return res;
}

/* validate pasted text on any server text control */
void  ServerTextValidator(ControlRef theControl)
 {
	Str255  theText,rText;
	Size   actualSize;
	UInt8  i,n;
	ControlID cid;
	OSStatus err=GetControlID(theControl,&cid);
	char *outText=NULL;
	printf("CIAO \n");
	// ................................. Get the text to be examined from the control

	GetControlData(theControl,kControlNoPart,kControlEditTextTextTag,
		sizeof(theText) -1,(Ptr) &theText,&actualSize);

	CarbonStream *me;
	err = GetControlProperty(theControl,CARBON_GUI_APP_SIGNATURE,
		SERVER_TEXT_PROPERTY,sizeof(CarbonStream *),NULL,&me);
		
	CarbonStreamServer *server = me->selectedServer();
	if(err!=noErr) error("Can't get CarbonStream pointer at ServerTextValidator (%d)!!",err);
	if(actualSize > 255) actualSize = 255;
	theText[actualSize]=0;
	switch(cid.id) {
		case PORT_CONTROL:
			n=0;
			for(i=0;i<=actualSize;i++) {
				if(theText[i] >= '0' && theText[i] <= '9') { 
					rText[n]=theText[i];
					n++;
				}
			}
			rText[n]=0;
			outText=(char *)rText;
			break;
		case URL_CONTROL:
			if(server) outText=server->url();
			//outText=
			break;
		default:
			outText=(char *)theText;
			break;
	}
	if(outText) 
		SetControlData(theControl,kControlNoPart,kControlEditTextTextTag,strlen(outText),(Ptr) outText);
	Draw1Control(theControl);
}