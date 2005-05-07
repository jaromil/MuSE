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

const ControlID streamTabID = { CARBON_GUI_APP_SIGNATURE, STREAM_TAB_CONTROL };
const ControlID serverTabID = { CARBON_GUI_APP_SIGNATURE, SERVER_TAB_CONTROL };

const ControlID serveLoginTypeID = { CARBON_GUI_APP_SIGNATURE, LOGIN_TYPE_CONTROL };
const ControlID serverStatusID = { CARBON_GUI_APP_SIGNATURE, SERVER_STATUS_CONTROL };

#define STREAM_EVENTS 1
const EventTypeSpec windowEvents[] = {
	{ kEventClassWindow, kEventWindowClose }
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
	else { /* new tab must be created */
		if(addTab(STREAM_TAB_CONTROL)) {
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
		for(int i=numServers;i>0;i--) {
			deleteServer(idx,i);
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
	int oldStreamIndex=getTabValue(STREAM_TAB_CONTROL,_selectedStream)-1;
	int oldServerIndex=getTabValue(SERVER_TAB_CONTROL,_selectedServer)-1;
		
	int sNum=0;
	_selectedServer=1;
	if(IsControlVisible(serverTabControl)) {
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
		err=GetControlData(control,0,kControlEditTextTextTag,SERVER_STRING_BUFFER_LEN,buffer,NULL);\
		if(err!=noErr) msg->error("Can't get __name from text control (%d)!!",err);\
	}
#define SAVE_SERVER_TEXT_INFO(__id,__name,__func) \
	{\
		SAVE_SERVER_INFO(__id,__name) \
		server->__func(buffer);\
		*buffer=0;\
	}
#define SAVE_SERVER_INT_INFO(__id,__name,__func) \
	{\
		SAVE_SERVER_INFO(__id,__name) \
		intBuffer=0;\
		sscanf(buffer,"%d",&intBuffer);\
		server->__func(intBuffer);\
		intBuffer=0;\
	}
	
void CarbonStream::saveServerInfo(CarbonStreamServer *server) {
	ControlRef control;	
	OSStatus err;
	int intBuffer =0;
	if(!server) return;
	char buffer[SERVER_STRING_BUFFER_LEN];
	ControlID cid;

	cid.signature=CARBON_GUI_APP_SIGNATURE;
	*buffer=0;
	
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
		err=SetControlData(control,0,kControlEditTextTextTag,len,buffer);\
		if(err!=noErr) msg->warning("Can't set __name from text control (%d)!!",err);\
		buffer=NULL;\
	}

void CarbonStream::updateServerInfo(CarbonStreamServer *server) {
	ControlRef control;
	ControlID cid;
	int len;
	OSStatus err;
	char *buffer=NULL;
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
	
}

/****************************************************************************/
/* EVENT HANDLERS */
/****************************************************************************/

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
    CarbonStream *me = (CarbonStream *)userData;
	err = GetEventParameter (event, kEventParamDirectObject,
        typeHICommand, NULL, sizeof(HICommand), NULL, &command);
    if(err != noErr) me->msg->error("Can't get event parameter!!");
	switch (command.commandID)
    {
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
		default:
			err = eventNotHandledErr;
			break;
	}
	return err;
}