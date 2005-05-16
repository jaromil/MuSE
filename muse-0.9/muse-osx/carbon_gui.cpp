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

#include <stdlib.h>
#include "carbon_gui.h"

#include <jmixer.h>
#include <jutils.h>
#include <config.h>
#include <unistd.h>


/* HANDLED EVENTS */
const EventTypeSpec vumeterEvents[] = {
	{ kEventClassWindow, kEventWindowClose }
};

const EventTypeSpec statusEvents[] = {
	{ kEventClassWindow, kEventWindowClose }
};

const EventTypeSpec events[] = {
	{ kEventClassWindow, kEventWindowClosed },
	{ CARBON_GUI_EVENT_CLASS, CARBON_GUI_REMOVE_CHANNEL},
	{ kEventClassWindow, kEventWindowGetClickActivation },
	{ kEventClassWindow, kEventWindowActivated }
};
	/* HANDLED COMMANDS */
const EventTypeSpec commands[] = {
	{ kEventClassCommand, kEventCommandProcess }
};

const ControlID mainControlsID[MAIN_CONTROLS_NUM] = {
	{ CARBON_GUI_APP_SIGNATURE, STREAM_BUT_ID },  
	{ CARBON_GUI_APP_SIGNATURE, NEWCH_BUT_ID },
	{ CARBON_GUI_APP_SIGNATURE, SNDOUT_BUT_ID }, 
	{ CARBON_GUI_APP_SIGNATURE, SNDIN_BUT_ID },
	{ CARBON_GUI_APP_SIGNATURE, VOL_BUT_ID },
	{ CARBON_GUI_APP_SIGNATURE, STATUS_BUT_ID },
	{ CARBON_GUI_APP_SIGNATURE, ABOUT_BUT_ID }
};

ControlRef mainControls[MAIN_CONTROLS_NUM];


static OSStatus MainWindowCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);
static OSStatus MainWindowEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);
static OSStatus VumeterWindowEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);
static OSStatus StatusWindowCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);
static OSStatus StatusWindowEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);

CARBON_GUI::CARBON_GUI(int argc, char **argv, Stream_mixer *mix) 
 : GUI(argc,argv,mix) {
  	jmix = mix;
	memset(myLcd,0,sizeof(myLcd));
	memset(myPos,0,sizeof(myPos));
	vumeter=0;
	vuband=0;
	/* by default we want at leat one active channel */
	if(!mix->chan[0]) mix->create_channel(0);
	// Create a Nib reference 
    err = CreateNibReference(CFSTR("main"), &nibRef);
	if(err != noErr) error("Can't get NIB reference to obtain gui controls!!");
    
	// Create the MainWindow using nib resource file
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
	if(err != noErr) error("Can't create MainWindow!!");
	else {
		msg = new CarbonMessage(nibRef);
		BringToFront(window);
		init_controls();
	
		// The window was created hidden so show it.
		ShowWindow( window );
		
		/* install vumeter controls */
		setupVumeters();
		setupStatusWindow();
		err=CreateWindowGroup(kWindowGroupAttrMoveTogether|kWindowGroupAttrLayerTogether|
			kWindowGroupAttrSharedActivation|kWindowGroupAttrHideOnCollapse,&mainGroup);
		err=SetWindowGroup(window,mainGroup);
		err=SetWindowGroup(vumeterWindow,mainGroup);
		err=SetWindowGroup(statusWindow,mainGroup);
	//	SetWindowGroupOwner(mainGroup,window);
		/* let's create a channel window for each active input channel */
		unsigned int i;
		for (i=0;i<MAX_CHANNELS;i++) {
			strcpy(ch_lcd[i],"00:00:00");
			if(jmix->chan[i]) { 
					CarbonChannel *newChan = new CarbonChannel(jmix,this,nibRef,i);
					channel[i] = newChan;
				/*	
					if(i > 0) {
						RepositionWindow(channel[i]->window,channel[i-1]->window,kWindowCascadeOnParentWindow);
					}
					else {
						RepositionWindow(channel[i],window,kWindowCascadeOnParentWindow);
					}
				*/
			}
			else {
				channel[i] = NULL;
			}
		}
		/* Ok, once MainWindow has been created and we have instantiated all acrive input channels,
		* we need an instance of CarbonStream to control the stream option window */
		streamHandler = new CarbonStream(jmix,window,nibRef);
	}
}

CARBON_GUI::~CARBON_GUI() { 
// We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
}

void CARBON_GUI::setupStatusWindow() {
	OSStatus err=CreateWindowFromNib(nibRef,CFSTR("StatusWindow"),&statusWindow);
	if(err!=noErr) msg->error("Can't create status window (%d)!!",err);
	SetDrawerParent(statusWindow,window);
	SetDrawerPreferredEdge(statusWindow,kWindowEdgeBottom);
	SetDrawerOffsets(statusWindow,20,20);
	err = InstallWindowEventHandler (statusWindow, 
		NewEventHandlerUPP (StatusWindowEventHandler), 
		GetEventTypeCount(statusEvents), statusEvents, this, NULL);
	if(err != noErr) msg->error("Can't install vumeter eventHandler");
	err=InstallWindowEventHandler(statusWindow,NewEventHandlerUPP(StatusWindowCommandHandler),
		GetEventTypeCount(commands),commands,this,NULL);
}

void CARBON_GUI::setupVumeters() {
	/* instance vumeters window that will be used later if user request it */
	OSStatus err=CreateWindowFromNib(nibRef, CFSTR("VumeterWindow"),&vumeterWindow);
	SetDrawerParent(vumeterWindow,window);
	SetDrawerPreferredEdge(vumeterWindow,kWindowEdgeTop);
	SetDrawerOffsets(vumeterWindow,20,20);
	if(err!=noErr) msg->error("Can't create vumeter window");
	/* install vmeter event handler+ */
	err = InstallWindowEventHandler (vumeterWindow, 
		NewEventHandlerUPP (VumeterWindowEventHandler), 
		GetEventTypeCount(vumeterEvents), vumeterEvents, this, NULL);
	if(err != noErr) msg->error("Can't install vumeter eventHandler");
}

void CARBON_GUI::showStreamWindow() {
	streamHandler->show();
}
void CARBON_GUI::run() {
	int i;
	int o = 0;
	UInt32 finalTicks;
	while(!quit) {
		lock(); /* lock before iterating on channel array ... if all is sane
				 * nobody can modify the channel list while we are managing it */
		for(i=0;i<MAX_CHANNELS;i++) {
			if(channel[i]) {
				if(new_pos[i]) {
					int newPos = (int)(ch_pos[i]*1000);
					channel[i]->setPos(newPos);
					new_pos[i] = false;
				}
				if(new_lcd[i]) { 
					channel[i]->setLCD(ch_lcd[i]);
					new_lcd[i] = false;
				}
				channel[i]->run();
				//QDFlushPortBuffer(GetWindowPort(channel[i]->window),NULL);
			}
		}
		unlock();
		if(meterShown()) updateVumeters();
		Delay(2,&finalTicks);
	}
 }

void CARBON_GUI::updateVumeters() {
	ControlID cid= { CARBON_GUI_APP_SIGNATURE , 0 };
	ControlRef control;
	SInt32 val;
	OSStatus err;
	/* volume */
	cid.id=VUMETER_VOL;
	err=GetControlByID(vumeterWindow,&cid,&control);
	if(err!=noErr) msg->error("Can't get vbar control (%d)!!",err);
	val=GetControl32BitValue(control);
	if(val!=vumeter) {
		char vdescr[256];
		SetControl32BitValue(control,vumeter);
		cid.id=VUMETER_VOL_DESCR;
		err=GetControlByID(vumeterWindow,&cid,&control);
		if(err!=noErr) msg->error("Can't get volume descr control (%d)!!",err);
		sprintf(vdescr,"%d",vumeter);
		err=SetControlData(control,0,kControlStaticTextTextTag,strlen(vdescr),vdescr);
	}
	
	/* bitrate */
	cid.id=VUMETER_BITRATE;
	err=GetControlByID(vumeterWindow,&cid,&control);
	if(err!=noErr) msg->error("Can't get vbar control (%d)!!",err);
	val=GetControl32BitValue(control);
	if(val!=vuband) {
		char bpsdescr[256];
		SetControl32BitValue(control,vuband);
		cid.id=VUMETER_BITRATE_DESCR;
		err=GetControlByID(vumeterWindow,&cid,&control);
		if(err!=noErr) msg->error("Can't get bps descr control (%d)!!",err);
		if(vuband<1000) {
			sprintf(bpsdescr,"%d B/s",vuband);
		}
		else if(vuband>1000 && vuband <1000000) {
			sprintf(bpsdescr,"%d KB/s",vuband/1000);
		}
		else {
			sprintf(bpsdescr,"%d MB/s",vuband/1000000);
		}
		err=SetControlData(control,0,kControlStaticTextTextTag,strlen(bpsdescr),bpsdescr);
	}
}

bool CARBON_GUI::new_channel() {
	int i;
	lock();
	for (i=0;i<MAX_CHANNELS;i++) {
		if(channel[i] == NULL) {
			if(new_channel(i)) {
				unlock();
				return true;
			}
		}
	}
	unlock();
	msg->warning("Actually MuSE doesn't support more than %d parallel input channels",MAX_CHANNELS);
	return false;
}

bool CARBON_GUI::new_channel(int i) {
	/* this is a private method....locking is managed by generic one new_channel() ... beware 
	 * if you use this method directly ... you will have to manage locks */
	if(i > MAX_CHANNELS) {
		msg->warning("Actually MuSE doesn't support more than %d concurrent input channels",MAX_CHANNELS);
		return false;
	}
	if(jmix->chan[i]) {
		msg->warning("Channel %d already exists :/",i);
	}
	else {
		if(!jmix->create_channel(i)) {
			msg->warning("Can't create new mixer channel %d",i);
		}
		CarbonChannel *newChan = new CarbonChannel(jmix,this,nibRef,i);
		channel[i] = newChan;
	//	if(i > 0) {
	//		RepositionWindow(channel[i]->window,channel[i-1]->window,kWindowCascadeOnParentWindow);
	//	}
		notice("created channel %d",i);
		return true;
	}
	return false;
}

bool CARBON_GUI::remove_channel(int idx) {
	lock();
	if(channel[idx]) {
		delete channel[idx];
		channel[idx] = NULL;
		unlock(); /* unlock mutex asap */
		jmix->stop_channel(idx);
		jmix->delete_channel(idx);
		notice("deleted channel %d",idx);
		return true;
	}
	unlock();
	return false;
}

 

void CARBON_GUI::set_title(char *txt) {
	CFStringRef title = CFStringCreateWithCString(NULL,txt,0);
	SetWindowTitleWithCFString (window,title);
	CFRelease(title);
}
 
void CARBON_GUI::set_status(char *txt) {
	HIViewRef statusTextView;
	TXNObject statusText;
	const ControlID txtid={ CARBON_GUI_APP_SIGNATURE, STATUS_TEXT_ID };
	err= HIViewFindByID(HIViewGetRoot(statusWindow), txtid, &statusTextView);
	if(err!=noErr) msg->warning("Can't get textView for status window (%d)!!",err);
	statusText = HITextViewGetTXNObject(statusTextView);
	if(!statusText) {
		msg->error("Can't get statusText object from status window!!");
	}
	if(txt) {
		TXNSetData(statusText,kTXNTextData,"[*] ",4,kTXNEndOffset,kTXNEndOffset);
		TXNSetData(statusText,kTXNTextData,txt,strlen(txt),kTXNEndOffset,kTXNEndOffset);
		TXNSetData(statusText,kTXNTextData,"\n",1,kTXNEndOffset,kTXNEndOffset);
	}
}

void CARBON_GUI::add_playlist(unsigned int ch, char *txt) {
	if(channel[ch]) channel[ch]->plUpdate();
}

void CARBON_GUI::sel_playlist(unsigned int ch, int row) {
	lock();
	if(channel[ch]) {
		channel[ch]->plSelect(row);
	}
	unlock();
}

bool CARBON_GUI::meterShown() { 
	return IsWindowVisible(vumeterWindow);
}

bool CARBON_GUI::init_controls() {
	int i;
	for(i=0;i<MAIN_CONTROLS_NUM;i++) {
		err = GetControlByID(window,&mainControlsID[i],&mainControls[i]);
		if(err != noErr) {
		//	printf("%d - %d - %d \n",i,mainControlsID[i].id,err);
			msg->error("Can't get control for button %d (%d)",i,err);
		}
	}
	
	/* By default start with live output enabled */
	jmix->set_lineout(true);
	SetControlValue(mainControls[SNDOUT_BUT],1);

	/* install main event handler+ */
	err = InstallWindowEventHandler (window, 
            NewEventHandlerUPP (MainWindowEventHandler), 
            GetEventTypeCount(events), events, 
            this, NULL);
	if(err != noErr) msg->error("Can't install main eventHandler");
	
	/* install main command handler */
    err = InstallWindowEventHandler (window, 
            NewEventHandlerUPP (MainWindowCommandHandler), 
            GetEventTypeCount(commands), commands, 
            this, NULL);
	if(err != noErr) msg->error("Can't install main commandHandler");
}

bool CARBON_GUI::attract_channels(int chIndex,AttractedChannel *neigh) {
	Rect bounds;
	if(!channel[chIndex]) return false;
	GetWindowBounds(channel[chIndex]->window,kWindowGlobalPortRgn,&bounds);
	for ( int i = 0;i < MAX_CHANNELS;i++) {
		if(i!=chIndex) {
			if(channel[i]) {
				Rect space;
				GetWindowBounds(channel[i]->window,kWindowStructureRgn,&space);
				if(!channel[i]->attached() && bounds.top > space.top-50 && bounds.top < space.top+50) {
					if(bounds.right > space.left-155 && bounds.right < space.left-90) {
						neigh->position=ATTACH_RIGHT;
						neigh->channel = channel[i];
						return true;
					}
					if(bounds.left < space.right+155 && bounds.left > space.right+90) {
						neigh->position=ATTACH_LEFT;
						neigh->channel=channel[i];
						return true;
					}
				}
			}
		}
	}
	return false;
}

void CARBON_GUI::showVumeters(bool flag) {
	OSStatus err;
	if(flag) {
		OpenDrawer(vumeterWindow,kWindowEdgeTop,false);
	}
	else {
		CloseDrawer(vumeterWindow,false);
	//	HideWindow(vumeterWindow);
	}
}

void CARBON_GUI::showStatus(bool flag) {
	OSStatus err;
	if(flag) { 
		OpenDrawer(statusWindow,kWindowEdgeBottom,false);

	}
	else {
		CloseDrawer(statusWindow,false);
	}
}

void CARBON_GUI::toggleStatus() {
	ToggleDrawer(statusWindow);
}

void CARBON_GUI::toggleVumeters() {
	ToggleDrawer(vumeterWindow);
}

void CARBON_GUI::clearStatus() {
}



/* END OF CARBON_GUI */


/* COMMAND HANDLER */
static OSStatus StatusWindowCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
	HICommand command; 
    OSStatus err = noErr;
	SInt16 val;
    CARBON_GUI *me = (CARBON_GUI *)userData;
	err = GetEventParameter (event, kEventParamDirectObject,
        typeHICommand, NULL, sizeof(HICommand), NULL, &command);
    if(err != noErr) me->msg->error("Can't get event parameter!!");
	switch (command.commandID)
    {
		case CLOSE_STATUS_CMD:
			me->showStatus(false);
			break;
		case CLEAR_STATUS_CMD:
			me->clearStatus();
			break;
		default:
            err = eventNotHandledErr;
            break;
	}
	return err;
}

static OSStatus MainWindowCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    HICommand command; 
    OSStatus err = noErr;
	SInt16 val;
    CARBON_GUI *me = (CARBON_GUI *)userData;
	err = GetEventParameter (event, kEventParamDirectObject,
        typeHICommand, NULL, sizeof(HICommand), NULL, &command);
    if(err != noErr) me->msg->error("Can't get event parameter!!");
	switch (command.commandID)
    {
        case 'sndo':
			val = GetControlValue(mainControls[SNDOUT_BUT]);
            me->jmix->set_lineout(val?true:false);
            break;
		case 'newc':
			me->new_channel();
			break;
		case SHOW_STREAMS_CMD:
				me->showStreamWindow();
			break;
		case 'vol ':
		//	val = GetControlValue(mainControls[VOL_BUT]);
		//	me->showVumeters(val?true:false);
			me->toggleVumeters();
			break;
		case SHOW_STATUS_CMD:
		//	val = GetControlValue(mainControls[STATUS_BUT]);
		//	me->showStatus(val?true:false);
			me->toggleStatus();
			break;
		case 'sndi':
		case 'abou':
        default:
            err = eventNotHandledErr;
            break;
    }
    return err;
}

/* EVENT HANDLER */
static OSStatus StatusWindowEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    OSStatus err = noErr;
    CARBON_GUI *me = (CARBON_GUI *)userData;
	switch (GetEventKind (event))
    {
		case kEventWindowClose:
			me->showStatus(false);
			break;
		default:
            break;
    }
    return err;
}

static OSStatus VumeterWindowEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    OSStatus err = noErr;
    CARBON_GUI *me = (CARBON_GUI *)userData;
	switch (GetEventKind (event))
    {
		case kEventWindowClose:
			SetControlValue(mainControls[VOL_BUT],0);
			me->showVumeters(false);
			break;
		default:
            err = eventNotHandledErr;
            break;
    }
    return err;
}

static OSStatus MainWindowEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    OSStatus err = noErr;
    CARBON_GUI *me = (CARBON_GUI *)userData;
	switch (GetEventKind (event))
    {
        case kEventWindowClosed: 
            QuitApplicationEventLoop();
            break;
		case kEventWindowGetClickActivation:  /* TODO - propagate activation click to the right control */
			/* XXX - still not handled */
			return CallNextEventHandler(nextHandler,event);
			break;
		case kEventWindowActivated:
			err = SetMenuBarFromNib(me->nibRef, CFSTR("MenuBar"));
			if(err != noErr) me->msg->error("Can't get MenuBar!!");
			break;
		case CARBON_GUI_REMOVE_CHANNEL:
			int idx;
			GetEventParameter(event,'cidx',typeCFIndex,NULL,sizeof(int),NULL,&idx);
			me->remove_channel(idx);
			break;
        default:
            err = eventNotHandledErr;
            break;
    }
    
    return err;

}
