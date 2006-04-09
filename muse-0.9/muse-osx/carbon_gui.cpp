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

#include <stdlib.h>
#include "carbon_gui.h"

#include <jmixer.h>
#include <jutils.h>
#include <config.h>
#include <unistd.h>

#define CGUI_STATUS_MAX_LINES 255

/* HANDLED EVENTS */

/* vumeter related events */
const EventTypeSpec vumeterEvents[] = {
	{ kEventClassWindow, kEventWindowClose }
};

/* status window related events */
const EventTypeSpec statusEvents[] = {
	{ kEventClassWindow, kEventWindowClose }
};

/* main events */
const EventTypeSpec events[] = {
	{ kEventClassWindow, kEventWindowClose },
	{ CARBON_GUI_EVENT_CLASS, CG_RMCH_EVENT},
	//{ kEventClassWindow, kEventWindowGetClickActivation },
	{ kEventClassWindow, kEventWindowActivated }
};

/* HANDLED COMMANDS */
const EventTypeSpec commands[] = {
	{ kEventClassCommand, kEventCommandProcess }
};

/* main window related commands*/
const ControlID mainControlsID[MAIN_CONTROLS_NUM] = {
	{ CARBON_GUI_APP_SIGNATURE, STREAM_BUT_ID },  
	{ CARBON_GUI_APP_SIGNATURE, NEWCH_BUT_ID },
	{ CARBON_GUI_APP_SIGNATURE, SNDOUT_BUT_ID }, 
	{ CARBON_GUI_APP_SIGNATURE, SNDIN_BUT_ID },
	{ CARBON_GUI_APP_SIGNATURE, VOL_BUT_ID },
	{ CARBON_GUI_APP_SIGNATURE, STATUS_BUT_ID },
	{ CARBON_GUI_APP_SIGNATURE, ABOUT_BUT_ID }
};

/* array of ControlRefs for all the buttons in the main window */
ControlRef mainControls[MAIN_CONTROLS_NUM];

/* prototypes for event and command handlers */
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

/* END of prototypes */

/* costants that can be used to set text and background colors */
const RGBColor white = {0xFFFF,0xFFFF,0xFFFF};
const RGBColor lgrey = {0xCCCC,0xCCCC,0xCCCC};
const RGBColor black = {0x0000,0x0000,0x0000 };


/* START of CARBON_GUI */

/* Constructor for CARBON_GUI class. */
CARBON_GUI::CARBON_GUI(int argc, char **argv, Stream_mixer *mix) 
 : GUI(argc,argv,mix) 
{
	/* initialization stuff */
  	jmix = mix;
	memset(myLcd,0,sizeof(myLcd));
	memset(myPos,0,sizeof(myPos));
	vumeter=0;
	vuband=0;
	selectedChannel=NULL;
	memset(channel,0,sizeof(channel));
	playlistManager=new PlaylistManager();
	msgList=new Linklist();
	
	/* init mutex used when accessing the statusbox buffer ...
	 * this is needed because other threads can try to write status messages concurrently
	 */
	if(pthread_mutex_init(&_statusLock,NULL) == -1) {
		error("error initializing POSIX thread mutex creating a new CarbonChannel");
		QuitApplicationEventLoop();
	}
	
	// Create a Nib reference 
    err = CreateNibReference(CFSTR("main"), &nibRef);
	if(err != noErr) error("Can't get NIB reference to obtain gui controls!!");
    
	// Create the MainWindow using nib resource file
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
	if(err != noErr) {
		error("Can't create MainWindow!!");
		QuitApplicationEventLoop();
	}
	else {
		msg = new CarbonMessage(nibRef);
		/* make the main window also the frontmost one */
		BringToFront(window);
		init_controls();
		
		/* now create the menu to use for the menubar ... it's stored in nib */
		err=CreateMenuFromNib(nibRef,CFSTR("MenuBar"),&mainMenu);
		if(err!=noErr) {
			msg->error("Can't create main menu (%d)!!",err);
		}
		
		/* install vumeter controls */
		setupVumeters();
		/* and the status box */
		setupStatusWindow();
		
		bufferInspector = new BufferInspector(window,nibRef,jmix);
		
		/* now we have to group windows together so if all are visible they will also be layered together */
		err=CreateWindowGroup(kWindowGroupAttrLayerTogether,&mainGroup);
		err=SetWindowGroup(window,mainGroup);
		err=SetWindowGroup(vumeterWindow,mainGroup);
		err=SetWindowGroup(statusWindow,mainGroup);
		err=SetWindowGroup(bufferInspector->window,mainGroup);
		SetWindowGroupOwner(mainGroup,window);
		/* let's create a channel window for each active input channel */
		unsigned int i;
		bool cc=false;
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
					cc=true;
			}
			else {
				channel[i] = NULL;
			}
		}
		/* Ok, once MainWindow has been created and we have instantiated all acrive input channels,
		* we need an instance of CarbonStream to control the stream option window */
		streamHandler = new CarbonStream(jmix,window,nibRef);
		/* by default we want at leat one active channel */
		if(!cc) new_channel();
	
		aboutWindow = new AboutWindow(window,nibRef);
		
		// The window was created hidden so show it.
		ShowWindow( window );
	}
}

/* destructor for the CARBON_GUI class 
 * here we destroy all other used objects and 
 */
CARBON_GUI::~CARBON_GUI() 
{ 
    /* Destroy used objects */
	delete streamHandler;
	delete playlistManager;
	delete aboutWindow;
	if(msgList) {
		while(msgList->len()) {
			Entry *e = msgList->begin();
			void *val = e->get_value();
			if(val) 
				free(val);
			delete e;
		}
		delete msgList;
	}
	/* delete all input channels */
	for (int i=0;i<MAX_CHANNELS;i++) 
		if(channel[i]) delete channel[i];
	DisposeWindow(statusWindow);
	DisposeWindow(vumeterWindow);
	DisposeMenu(mainMenu);
	// We don't need the nib reference anymore.
	DisposeNibReference(nibRef);
}

/* initialize the status window (used to show console messages in the graphic environment */
void CARBON_GUI::setupStatusWindow() 
{
	OSStatus err=CreateWindowFromNib(nibRef,CFSTR("StatusWindow"),&statusWindow);
	if(err!=noErr) msg->error("Can't create status window (%d)!!",err);
	//SetDrawerParent(statusWindow,window);
	//SetDrawerPreferredEdge(statusWindow,kWindowEdgeBottom);
	//SetDrawerOffsets(statusWindow,20,20);
	
	/* install an eventHandler to intercept close requests */
	err = InstallWindowEventHandler (statusWindow, 
		NewEventHandlerUPP (StatusWindowEventHandler), 
		GetEventTypeCount(statusEvents), statusEvents, this, NULL);
	if(err != noErr) msg->error("Can't install status window eventHandler");
	
	/* and then install a command handler (to handle "clear" requests) */
	err=InstallWindowEventHandler(statusWindow,NewEventHandlerUPP(StatusWindowCommandHandler),
		GetEventTypeCount(commands),commands,this,NULL);
		
	/* obtain an HIViewRef for the status text box ... we have to use it 
	 * to setup various properties and to obain a TXNObject needed to manage its content */
	HIViewRef statusTextView;
	const ControlID txtid={ CARBON_GUI_APP_SIGNATURE, STATUS_TEXT_ID };
	err= HIViewFindByID(HIViewGetRoot(statusWindow), txtid, &statusTextView);
	if(err!=noErr) return;// msg->warning("Can't get textView for status window (%d)!!",err);
	statusText = HITextViewGetTXNObject(statusTextView);
	if(!statusText) {
		msg->error("Can't get statusText object from status window!!");
	}

	/* setup status text font size and color */
	// Create type attribute data structure
	UInt32   fontSize = 10 << 16; // needs to be in Fixed format
	TXNAttributeData fsData,fcData;
	fsData.dataValue=fontSize;
	fcData.dataPtr=(void *)&black;
	TXNTypeAttributes attributes[] = {
		//{ kTXNQDFontStyleAttribute, kTXNQDFontStyleAttributeSize, bold },
		{ kTXNQDFontColorAttribute, kTXNQDFontColorAttributeSize,fcData}, //&lgrey },
		{ kTXNQDFontSizeAttribute, kTXNFontSizeAttributeSize,fsData }
	};
	err= TXNSetTypeAttributes( statusText, 2, attributes,
		kTXNStartOffset,kTXNEndOffset );
		
	/* block user input in the statusText box */
	TXNControlTag tags[] = { kTXNNoUserIOTag };
	TXNControlData vals[] = { kTXNReadOnly };
	err=TXNSetTXNObjectControls(statusText,false,1,tags,vals);
	if(err!=noErr) msg->error("Can't set statusText properties (%d)!!",err);
	// TXNSetScrollbarState(statusText,kScrollBarsAlwaysActive);
		
	//struct TXNBackground bg = {  kTXNBackgroundTypeRGB, black };
	//TXNSetBackground(statusText,&bg);
}

/* 
 * initialize controls for the vumeter window 
 */
void CARBON_GUI::setupVumeters() 
{
	/* instance vumeters window that will be used later if user request it */
	OSStatus err=CreateWindowFromNib(nibRef, CFSTR("VumeterWindow"),&vumeterWindow);
//	SetDrawerParent(vumeterWindow,window);
//	SetDrawerPreferredEdge(vumeterWindow,kWindowEdgeTop);
//	SetDrawerOffsets(vumeterWindow,20,20);
	if(err!=noErr) msg->error("Can't create vumeter window");
	/* install vmeter event handler+ */
	err = InstallWindowEventHandler (vumeterWindow, 
		NewEventHandlerUPP (VumeterWindowEventHandler), 
		GetEventTypeCount(vumeterEvents), vumeterEvents, this, NULL);
	if(err != noErr) msg->error("Can't install vumeter eventHandler");
}

/*
 * just show the stream window 
 */
void CARBON_GUI::showStreamWindow() 
{
	streamHandler->show();
}

/* 
 * main loop for the CarbonGui object
 */
void CARBON_GUI::run() {
	int i;
	int o = 0;
	//UInt32 finalTicks;
	while(!quit) {
		
		lock(); /* lock before iterating on channel array ... if all is sane
				 * nobody can modify the channel list while we are managing it */
		wait(); /* wait the tick signal from jmixer */
		/* now iterate an channels updating lcd and pos as necessary */
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
				channel[i]->run(); /* propagate tick on each channel */
				//QDFlushPortBuffer(GetWindowPort(channel[i]->window),NULL);
			}
		}
		if(playlistManager->isTouched()) 
			playlistManager->untouch(); /* reset playlistManager update flag */
		unlock();
		if(meterShown()) updateVumeters();
		if(statusText && msgList->len() > 0) {
			/*	
			statusLock();
			if(statusText && !quit) {
				err=TXNSetTypeAttributes(statusText,3,attributes,kTXNUseCurrentSelection,kTXNUseCurrentSelection);
				if(err!=noErr) msg->warning("Can't set status text attributes (%d)!!",err);
			}
			statusUnlock(); 
			*/ 
			while(msgList->len() > 0) {
				Entry *msg = msgList->begin();
				char *txt = (char *)msg->get_value();
				if(txt) {
					msgOut(txt);
					free(txt);
				}
				delete msg;
			}
		}
		//	Delay(2,&finalTicks); /* DISABLED BEACAUSE NOW TICK IS SIGNALED BY JMIXER */
		if(IsWindowVisible(bufferInspector->window))
			bufferInspector->run();
	}
 }

void CARBON_GUI::msgOut(char *txt) {
	ItemCount nLines=0;
	//SInt32 nCols=0;
	Handle oldText = NULL;
	TXNOffset start,end;
	char *p;
	int off;
	
	if(txt && statusText) {
		TXNGetLineCount(statusText,&nLines);
		if(nLines > CGUI_STATUS_MAX_LINES) {
			TXNSelectAll(statusText);
			TXNGetSelection(statusText,&start,&end);
			//TXNSetSelection(statusText,kTXNEndOffset,kTXNEndOffset);
			
			if(TXNGetDataEncoded(statusText,start,end,&oldText,kTXNTextData) != noErr)
				msg->error("Can't get status text buffer!! (%d)\n");
		
			p = *oldText;
			while(*p != '\n' && off < end-start) {
				p++;
				off++;
			}
			p++;
			off++;
			TXNSetData(statusText,kTXNTextData,p,(end-start)-off,kTXNStartOffset,kTXNEndOffset);
			//err = TXNScroll(statusText,kTXNScrollUnitsInLines,kTXNScrollUnitsInLines,&nLines,&nCols);
		}
		TXNSetData(statusText,kTXNTextData,"[*] ",4,kTXNEndOffset,kTXNEndOffset);
		TXNSetData(statusText,kTXNTextData,txt,strlen(txt),kTXNEndOffset,kTXNEndOffset);
		TXNSetData(statusText,kTXNTextData,"\n",1,kTXNEndOffset,kTXNEndOffset);
	}
	if(oldText)
		DisposeHandle(oldText);

}

/*
 * update vumeterWindow to reflect actual volume and bandwith information
 */
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
	if(val!=vumeter) 
	{
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
	if(val!=vuband) 
	{
		char bpsdescr[256];
		SetControl32BitValue(control,vuband);
		cid.id=VUMETER_BITRATE_DESCR;
		err=GetControlByID(vumeterWindow,&cid,&control);
		if(err!=noErr) msg->error("Can't get bps descr control (%d)!!",err);
		if(vuband<1000) 
			sprintf(bpsdescr,"%d B/s",vuband);
		else if(vuband>1000 && vuband <1000000) 
			sprintf(bpsdescr,"%d KB/s",vuband/1000);
		else 
			sprintf(bpsdescr,"%d MB/s",vuband/1000000);

		err=SetControlData(control,0,kControlStaticTextTextTag,strlen(bpsdescr),bpsdescr);
	}
}

bool CARBON_GUI::new_channel() 
{
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
		notice("created channel %d",i);
		return true;
	}
	return false;
}

bool CARBON_GUI::remove_channel(int idx) {
	lock();
	if(channel[idx]) {
		if(selectedChannel==channel[idx]) selectedChannel=NULL;
		delete channel[idx];
		channel[idx] = NULL;
		unlock(); /* unlock mutex asap (but we have to delete CarbonChannel object first */
		if(jmix->chan[idx]) {
			/* if still present, destroy the underlying muse channel object */
			jmix->stop_channel(idx);
			jmix->delete_channel(idx);
			notice("deleted channel %d",idx);
		}
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
	OSStatus err = noErr;
	Entry *newEntry;
	char *p;
	int off=0;
	
	if(!txt) 
		return;
	newEntry = new Entry((void *)strdup(txt));
	msgList->append(newEntry);
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

/* 
 * given an index and a pointer to an AttractedChannel structure, this routine
 * checks if there is another channel window in the neighbourhood and if it's there,
 * true is returned and neigh is filled with a reference to the neighbour channel and a flag
 * that specifies if its window is on the left or on the right
 */
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

void CARBON_GUI::toggleBufferInspector() {
	if(IsWindowVisible(bufferInspector->window)) showBufferInspector(false);
	else showBufferInspector(true);
}

void CARBON_GUI::showBufferInspector(bool flag) {
	if (flag) bufferInspector->show();
	else bufferInspector->hide();
}

/*
 * show or hide vumeter window as specified trough "flag" argument
 */
void CARBON_GUI::showVumeters(bool flag) {
	OSStatus err;
	if(flag) {
	//	OpenDrawer(vumeterWindow,kWindowEdgeTop,false);
		Rect bounds;
		GetWindowBounds(window,kWindowGlobalPortRgn,&bounds);
		MoveWindow(vumeterWindow,bounds.right+15,bounds.top-5,false);
		ShowWindow(vumeterWindow);
	}
	else {
	//	CloseDrawer(vumeterWindow,false);
		HideWindow(vumeterWindow);
	}
}

/*
 * show or hide status window as specified trough "flag" argument
 */
void CARBON_GUI::showStatus(bool flag) {
	OSStatus err;
	SInt32 nLines=0;
	SInt32 nCols=0;
	if(flag) { 
		Rect bounds;
		GetWindowBounds(window,kWindowGlobalPortRgn,&bounds);
		MoveWindow(statusWindow,bounds.left,bounds.bottom+30,false);
		ShowWindow(statusWindow);
		TXNGetLineCount(statusText,(ItemCount *)&nLines);
		//TXNSetData(statusText,kTXNTextData,"",0,kTXNEndOffset,kTXNEndOffset);
		TXNScroll(statusText,kTXNScrollUnitsInLines,kTXNScrollUnitsInLines,&nLines,&nCols);
		TXNForceUpdate(statusText);
		//OpenDrawer(statusWindow,kWindowEdgeBottom,false);
	}
	else {
		HideWindow(statusWindow);
		//CloseDrawer(statusWindow,false);
	}
}

/* 
 * toggle visibility of status window
 */
void CARBON_GUI::toggleStatus() {
	//ToggleDrawer(statusWindow);
	if(IsWindowVisible(statusWindow)) showStatus(false);
	else showStatus(true);
}

/* 
 * toggle visibility of vumeters window 
 */
void CARBON_GUI::toggleVumeters() {
//	ToggleDrawer(vumeterWindow);
	if(IsWindowVisible(vumeterWindow)) showVumeters(false);
	else showVumeters(true);
}

/*
 * clears the status window
 */
void CARBON_GUI::clearStatus() {
	TXNSetData(statusText,kTXNTextData,"",0,kTXNStartOffset,kTXNEndOffset);
}

/*
 * Bring all windows in main windowgroup to frontmost layer
 * we have to handle this explicitly because of Quartz behaviour 
 * when layering windowgroups
 */
void CARBON_GUI::bringToFront() {
	bool anyChannel=false;
	for ( int i = 0;i < MAX_CHANNELS;i++) {
		if(channel[i]) {
			anyChannel=true;
			SendWindowGroupBehind(channel[i]->faderGroup,mainGroup);
		}
	}
	if(!anyChannel) BringToFront(window);
	SelectWindow(window);
	OSStatus err = SetRootMenu(mainMenu);
	if(err != noErr) msg->error("Can't set MenuBar!!");
}

/*
 * Handle layering when a channel window is ativated (by clicking on it).
 * This stuff is needed because of Quartz behaviour when layering windowgroups
 */
void CARBON_GUI::activatedChannel(int idx) {
	if(channel[idx]) {
		if(selectedChannel && selectedChannel!=channel[idx]) {
			SendWindowGroupBehind(selectedChannel->faderGroup,channel[idx]->faderGroup);
		}
		selectedChannel=channel[idx];
	}	
}

/* 
 * show AboutWindow
 */
void CARBON_GUI::credits() {
	aboutWindow->show();
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
		case 'sndi':
			val = GetControlValue(mainControls[SNDIN_BUT]);
			me->jmix->set_live(val?true:false);
			break;
		case 'newc':
			me->new_channel();
			break;
		case SHOW_STREAMS_CMD:
			me->showStreamWindow();
			break;
		case SHOW_VUMETERS_CMD:
			me->toggleVumeters();
			break;
		case SHOW_STATUS_CMD:
			me->toggleStatus();
			break;
		case ABOUT_CMD:
			me->credits();
			break;
		case BI_SHOW:
			me->toggleBufferInspector();
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
        case kEventWindowClose: 
            QuitApplicationEventLoop();
            break;
	//	case kEventWindowGetClickActivation:  /* TODO - propagate activation click to the right control */
	//		/* XXX - still not handled */
	//		return CallNextEventHandler(nextHandler,event);
	//		break;
		case kEventWindowActivated:
			me->bringToFront();
			break;
		case CG_RMCH_EVENT: /* a channel window has been closed */
			int idx;
			GetEventParameter(event,CG_RMCH_EVENT_PARAM,typeCFIndex,NULL,sizeof(int),NULL,&idx);
			me->remove_channel(idx);
			break;
        default:
            err = eventNotHandledErr;
            break;
    }
    
    return err;

}
