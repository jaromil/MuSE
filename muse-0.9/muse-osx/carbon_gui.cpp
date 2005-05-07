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


/* HANDLED EVENTS */
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
	{ CARBON_GUI_APP_SIGNATURE, ABOUT_BUT_ID }
};

ControlRef mainControls[MAIN_CONTROLS_NUM];


static OSStatus MainWindowCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);
static OSStatus MainWindowEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);

CARBON_GUI::CARBON_GUI(int argc, char **argv, Stream_mixer *mix)
  : GUI(argc,argv,mix) {
  	jmix = mix;
	memset(myLcd,0,sizeof(myLcd));
	memset(myPos,0,sizeof(myPos));
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

void CARBON_GUI::showStreamWindow() {
	streamHandler->show();
}
void CARBON_GUI::run() {
	int i;
	int o = 0;
	while(!quit) {
		for(i=0;i<MAX_CHANNELS;i++) {
			lock();
			if(channel[i]) {
				if(new_pos[i]) {
					int newPos = (int)(ch_pos[i]*1000);
					//if(newPos != myPos[i]) {
						//myPos[i] = newPos;
						channel[i]->setPos(newPos);
						new_pos[i] = false;
					//}
				}
				if(new_lcd[i]) { 
				//	if(strcmp(ch_lcd[i],myLcd[i]) != 0) {
				//		strncpy(myLcd[i],ch_lcd[i],255);
						channel[i]->setLCD(ch_lcd[i]);
						new_lcd[i] = false;
				//	}
				}
			}
			unlock();
		}
		usleep(1000);
	//	jsleep(0,20);
	}
 }
 
bool CARBON_GUI::new_channel() {
	int i;
	for (i=0;i<MAX_CHANNELS;i++) {
		if(channel[i] == NULL) {
			if(new_channel(i)) return true;
		}
	}
	msg->warning("Actually MuSE doesn't support more than %d parallel input channels",MAX_CHANNELS);
	return false;
}

bool CARBON_GUI::new_channel(int i) {
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
	if(channel[idx]) {
		delete channel[idx];
		channel[idx] = NULL;
		jmix->stop_channel(idx);
		jmix->delete_channel(idx);
		notice("deleted channel %d",idx);
		return true;
	}
	return false;
}

 

void CARBON_GUI::set_title(char *txt) {
	CFStringRef title = CFStringCreateWithCString(NULL,txt,0);
	SetWindowTitleWithCFString (window,title);
}
 
void CARBON_GUI::set_status(char *txt) {
//  gtkgui_set_statustext(txt);
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

bool CARBON_GUI::meter_shown() { }

bool CARBON_GUI::init_controls() {
	int i;
	for(i=0;i<MAIN_CONTROLS_NUM;i++) {
		err = GetControlByID(window,&mainControlsID[i],&mainControls[i]);
		if(err != noErr) {
		//	printf("%d - %d - %d \n",i,mainControlsID[i].id,err);
			msg->error("Can't get control for button %d (%d)",i,err);
		}
	}
	
	/* Now simulate a click to the SNDOUT button to let start checked */
	HIViewRef kk;
	HIViewFindByID(HIViewGetRoot(window),mainControlsID[SNDOUT_BUT],&kk);
	ControlPartCode ka;
	err = HIViewSimulateClick(kk,kControlButtonPart,0,&ka);
	if(err != noErr) {
		/* TODO - Error messages */
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

/* END OF CARBON_GUI */


/* COMMAND HANDLER */
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
            if(val) {
				me->jmix->set_lineout(true);
			}
			else {
				me->jmix->set_lineout(false);
			}
            break;
		case 'newc':
			me->new_channel();
			break;
		case SHOW_STREAMS_CMD:
				me->showStreamWindow();
		case 'sndi':
		case 'vol ':
		case 'abou':
        default:
            err = eventNotHandledErr;
            break;
    }
    
CantGetParameter:
    return err;
}

/* EVENT HANDLER */
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
