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
 
#include "carbon_channel.h"
#include "carbon_gui.h"

#define WINDOW_GROUP_ATTRIBUTES \
	kWindowGroupAttrMoveTogether|kWindowGroupAttrLayerTogether|\
	kWindowGroupAttrSharedActivation|kWindowGroupAttrHideOnCollapse

extern "C" OSStatus OpenFileWindow(WindowRef parent);
extern "C" OSStatus OpenUrlWindow(WindowRef parent,IBNibRef nib);

/****************************************************************************/
/* Globals */
/****************************************************************************/

const ControlID selectedSongId = { CARBON_GUI_APP_SIGNATURE, SELECTED_SONG_CONTROL };
const ControlID volId = { CARBON_GUI_APP_SIGNATURE,VOLUME_CONTROL };

#define CARBON_CHANNEL_EVENTS 8
const EventTypeSpec windowEvents[] = {
        { kEventClassWindow, kEventWindowActivated },
		{ kEventClassWindow, kEventWindowGetClickActivation },
		{ kEventClassWindow, kEventWindowClosed },
		{ kEventClassWindow, kEventWindowBoundsChanging },
		{ kEventClassWindow, kEventWindowDragCompleted },
		{ kEventClassWindow, kEventWindowResizeStarted },
		{ kEventClassWindow, kEventWindowResizeCompleted },
		{ kCoreEventClass, kAEOpenDocuments }
};

#define DATA_BROWSER_EVENTS 2
const EventTypeSpec dataBrowserEvents[] = {
		{kEventClassControl, kEventControlActivate},
		{kEventClassControl, kEventControlDragEnter},
	//	{kEventClassMouse,kEventMouseDown}
};

const EventTypeSpec windowCommands[] = {
	{ kEventClassCommand, kEventCommandProcess }
};

/* Start of CarbonChannel */

CarbonChannel::CarbonChannel(Stream_mixer *mix,CARBON_GUI *gui,IBNibRef nib,unsigned int chan) {
	parent = gui;
	parentWin = parent->window;
	jmix = mix;
	nibRef = nib;
	chIndex = chan;
	OSStatus err;
	jmix->set_playmode(chIndex,PLAYMODE_PLAYLIST);
	inChannel = jmix->chan[chIndex];
	memset(&neigh,0,sizeof(neigh));
	isAttached=false;
	isSlave=false;
	isResizing=false;
	isDrawing=false;
	status=CC_STOP;
	savedStatus=-1;
	playList = inChannel->playlist; //new Playlist();
	
	msg = new CarbonMessage(nibRef);
	err = CreateWindowFromNib(nibRef, CFSTR("Channel"), &window);
	HISize minBounds = {CHANNEL_WINDOW_WIDTH_MIN,CHANNEL_WINDOW_HEIGHT_MIN};
	HISize maxBounds = {CHANNEL_WINDOW_WIDTH_MAX,CHANNEL_WINDOW_HEIGHT_MAX};
	SetWindowResizeLimits(window,&minBounds,&maxBounds);
	err = CreateMenuFromNib (nibRef,CFSTR("PLMenu"),&plMenu);
	if(err != noErr) {
		msg->error("Can't create plMenu ref (%d)!!",err);
	}
	err = CreateMenuFromNib (nibRef,CFSTR("PLEntryMenu"),&plEntryMenu);
	if(err != noErr) {
		msg->error("Can't create plMenu ref (%d)!!",err);
	}
	
	err = InstallWindowEventHandler(window,ChannelEventHandler,CARBON_CHANNEL_EVENTS,
		windowEvents,this,NULL);
	if(err != noErr) { 
		msg->error("Can't install event handler for Channel control (%d)!!",err);
	}
	/* install the channel command handler */
    err = InstallWindowEventHandler (window, 
            NewEventHandlerUPP (channelCommandHandler), 
            GetEventTypeCount(windowCommands), windowCommands, 
            this, NULL);
	if(err != noErr) msg->error("Can't install channel commandHandler");

	
	CFStringRef format = CFStringCreateWithCString(NULL,"Channel %d",0);
	CFStringRef wName = CFStringCreateWithFormat(NULL,NULL,format,chan);
	SetWindowTitleWithCFString (window,wName);
	CFRelease(format);
	CFRelease(wName);
	/* and finally we can show the channelwindow */
	ShowWindow(window);
	ControlRef volCtrl;
	err = GetControlByID(window,&volId,&volCtrl);
	SetControlValue(volCtrl,(int)(inChannel->volume*100));
	EventLoopTimerUPP timerUPP = NewEventLoopTimerUPP(channelLoop);
	err = InstallEventLoopTimer( GetCurrentEventLoop(),0,1,timerUPP,this,&updater);
	if(err!=noErr) msg->error("Can't install the idle eventloop handler(%d)!!",err);

	/* install fader window */
	setupFaderWindow();
	err=CreateWindowGroup(WINDOW_GROUP_ATTRIBUTES,&faderGroup);
	if(err!=noErr) msg->warning("%d",err);
	err=SetWindowGroup(window,faderGroup);
	err=SetWindowGroup(fader,faderGroup);
	BringToFront(window);
	
	//SetWindowGroupOwner(faderGroup,window);


	/* setup playList control */
	plSetup();

	SetAutomaticControlDragTrackingEnabledForWindow (window, true);

	setupOpenUrlWindow();
}

CarbonChannel::~CarbonChannel() {
	/* remove the eventloop associated to our window */
//	AERemoveEventHandler (kCoreEventClass,kAEOpenDocuments,openHandler,false);
	RemoveEventLoopTimer(updater);
	RemoveEventHandler(windowEventHandler);
	RemoveEventHandler(playListEventHandler);
	
	/* remove some local structures */
	DisposeMenu(plMenu);
	DisposeMenu(plEntryMenu);
	delete msg;
	RemoveControlProperty(playListControl,CARBON_GUI_APP_SIGNATURE,PLAYLIST_PROPERTY);
	DisposeWindow(fader);
	ReleaseWindowGroup(faderGroup);
	DisposeWindow(openUrl);
	/* TODO - maybe more cleaning is needed */
}

void CarbonChannel::setupFaderWindow() {
	const ControlID faderId = { CARBON_GUI_APP_SIGNATURE,FADER_ID };
	OSStatus err = CreateWindowFromNib(nibRef,CFSTR("FaderWindow"),&fader);
	if(err!=noErr) msg->error("Can't create fader drawer (%d)!!",err);
	SetDrawerParent(fader,window);
	SetDrawerPreferredEdge(fader,kWindowEdgeRight);
	SetDrawerOffsets(fader,50,50); /* XXX - HC offsets ... should go in a #define */
	err = InstallWindowEventHandler (fader, 
            NewEventHandlerUPP (faderCommandHandler), 
            GetEventTypeCount(windowCommands),windowCommands, 
            this, NULL);
			
	err = GetControlByID(fader,&faderId,&faderControl);
	if(err != noErr) {
		msg->error("Can't obtain dataBrowser ControlRef (%d)!!",err);
	}
	CarbonChannel *self=this;
	err = SetControlProperty(faderControl,CARBON_GUI_APP_SIGNATURE,FADER_PROPERTY,
		sizeof(CarbonChannel *),&self);
	if(err!=noErr) msg->error("Can't attach CarbonChannel object to Fader control (%d) !!",err);
}

void CarbonChannel::setupOpenUrlWindow() {
	OSStatus err=CreateWindowFromNib(nibRef, CFSTR("AddURLWindow"), &openUrl);
	if(err != noErr) msg->error("Can't create the openUrl window (%d)!!",err);
	/* install the channel command handler */
	err = InstallWindowEventHandler (openUrl,NewEventHandlerUPP (openUrlCommandHandler), 
		GetEventTypeCount(windowCommands), windowCommands, this, NULL);
	if(err != noErr) msg->error("Can't install openUrl commandHandler");
}

void CarbonChannel::plSetup() {
	OSStatus err;
	const ControlID dataBrowserId = { CARBON_GUI_APP_SIGNATURE, PLAYLIST_BOX_ID };
	err = GetControlByID(window,&dataBrowserId,&playListControl);
	if(err != noErr) {
		msg->error("Can't obtain dataBrowser ControlRef (%d)!!",err);
	}
	EventTargetRef dbTarget = GetControlEventTarget (playListControl);
	
	CarbonChannel *self=this;
	err = SetControlProperty(playListControl,CARBON_GUI_APP_SIGNATURE,PLAYLIST_PROPERTY,
		sizeof(CarbonChannel *),&self);
	if(err!=noErr) msg->error("Can't attach CarbonChannel object to Playlist control (%d) !!",err);	
	
	/* installs databrowser event handler */
	err = InstallEventHandler(dbTarget,dataBrowserEventHandler,DATA_BROWSER_EVENTS,dataBrowserEvents,
		this,&playListEventHandler);

	DataBrowserCallbacks  dbCallbacks;
	/* installs databrowser callbacks */
	dbCallbacks.version = kDataBrowserLatestCallbacks; 
    InitDataBrowserCallbacks (&dbCallbacks); 

	/* main callback */
    dbCallbacks.u.v1.itemDataCallback=NewDataBrowserItemDataUPP(HandlePlaylist); 
	/* callback to check if we have to accept a drag */
	dbCallbacks.u.v1.acceptDragCallback=NewDataBrowserAcceptDragUPP(&CheckDrag);
	/* Drag handler */
	dbCallbacks.u.v1.receiveDragCallback=NewDataBrowserReceiveDragUPP(&HandleDrag);
	/* Notification Handler */
	dbCallbacks.u.v1.itemNotificationCallback=
		NewDataBrowserItemNotificationUPP(&HandleNotification);
	/* context menu handler */
	dbCallbacks.u.v1.getContextualMenuCallback=
		NewDataBrowserGetContextualMenuUPP(&GetPLMenu);
//	dbCallbacks.u.v1.selectContextualMenuCallback=
//		NewDataBrowserSelectContextualMenuUPP(&SelectPLMenu);
	/* drag starter */
	dbCallbacks.u.v1.addDragItemCallback=NewDataBrowserAddDragItemUPP(&AddDrag);
	
	/* custom callbacks */
//	DataBrowserCustomCallbacks  dbCustomCallbacks;
//	InitDataBrowserCustomCallbacks(&dbCustomCallbacks);
	/* Tracking Handler */
//	dbCustomCallbacks.u.v1.trackingCallback=NewDataBrowserTrackingUPP(&PlaylistTracking);	
	
	/* register callbacks */
	SetDataBrowserCallbacks(playListControl, &dbCallbacks);
	
	SetControlDragTrackingEnabled(playListControl,true);
}

bool CarbonChannel::plUpdate() {
	lock();
	int i;
	int len = playList->len();
	DataBrowserItemID idList[len];
	RemoveDataBrowserItems(playListControl,kDataBrowserNoItem,0,NULL,kDataBrowserItemNoProperty);
	for(i=1;i<=len;i++) {
	Url *entry = (Url *)playList->pick(i);
		idList[i-1] = i;
	}
	AddDataBrowserItems(playListControl,kDataBrowserNoItem,len,idList,kDataBrowserItemNoProperty);
	
	/* update selected song */
	ControlRef textControl;
	OSStatus err = GetControlByID(window,&selectedSongId,&textControl);
	if(err != noErr) {
		msg->error("Can't get selectedSong control ref (%d)!!",err);
	}
	Url *entry = (Url *)playList->selected();
	if(entry) {
		/* XXX - lazy coding */
		char *p = entry->path+strlen(entry->path);
		while(*p!='/') {
			if(p==entry->path) break;
			p--;
		}
		if(*p=='/') p++;
		err=SetControlData(textControl, 0, kControlStaticTextTextTag,strlen(p), p);
	}
	unlock();
}

bool CarbonChannel::plAdd(char *txt) {
	lock();
	if(txt)	return jmix->add_to_playlist(chIndex,txt);
	unlock();
	return false;
}

void CarbonChannel::close () {
	EventRef event;
	OSStatus err;
	err = CreateEvent (NULL,CARBON_GUI_EVENT_CLASS,CG_RMCH_EVENT,0,kEventAttributeUserEvent,&event);
	if(err != noErr) msg->error("Can't create rmCh event!!");
	SetEventParameter(event,CG_RMCH_EVENT_PARAM,typeCFIndex,sizeof(int),&chIndex);
	err = SendEventToEventTarget(event,GetWindowEventTarget(parentWin));
	if(err != noErr) {
		msg->error("Can't send rmCh event to mainWin!!");
	}
	if(isAttached) 	stopFading();
	//delete me;
}

MenuRef CarbonChannel::plGetMenu() {
	int i;
	DataBrowserItemID first,last;
	GetDataBrowserSelectionAnchor(playListControl,&first,&last);
	if(first) {
		return plEntryMenu;
	}
	return plMenu;
}

void CarbonChannel::activateMenuBar() {
	OSStatus err;
	err = SetMenuBarFromNib(nibRef, CFSTR("PLMenu"));
	if(err != noErr) msg->error("Can't get MenuBar!!");
}

void CarbonChannel::setLCD(char *text) {
	OSStatus err;
	ControlRef lcdControl;
	const ControlID seekTimeId = { CARBON_GUI_APP_SIGNATURE, SEEK_TIME_CONTROL };
	if(strncmp(text,lcd,255) != 0) {
		strncpy(lcd,text,255);
		GetControlByID(window,&seekTimeId,&lcdControl);
		SetControlData (lcdControl, 0, kControlStaticTextTextTag,strlen(text), text);
	}
}

void CarbonChannel::setPos(int pos) {
	OSStatus err;
	ControlRef seekControl;
	const ControlID seekId = { CARBON_GUI_APP_SIGNATURE, SEEK_CONTROL };
	if(pos!=seek) {
		err = GetControlByID(window,&seekId,&seekControl);
		if(err != noErr) msg->warning("Can't update seek control (%d)!!",err);
		SetControl32BitValue(seekControl,pos);
		seek = pos;
	}
}

void CarbonChannel::plSelect(int row) {
	lock();
	OSStatus err;
	if(row) {
		if(playList->selected_pos()!=row) {
			inChannel->sel(row);
		}
	}
	unlock();
}

void CarbonChannel::updateSelectedSong(int row) {
	ControlRef textControl;
	OSStatus err=noErr;
	err = GetControlByID(window,&selectedSongId,&textControl);
	if(err != noErr) {
		msg->error("Can't get selectedSong control ref (%d)!!",err);
	}
	if(row) {
		Url *entry = (Url *)playList->pick(row);
		if(entry) {
			/* XXX - lazy coding */
			char *p = entry->path+strlen(entry->path);
			while(*p!='/') {
				if(p==entry->path) break;
				p--;
			}
			if(*p=='/') p++;
			err=SetControlData (textControl, 0, kControlStaticTextTextTag,strlen(p), p);
		}
	}
	else {
		err=SetControlData (textControl, 0, kControlStaticTextTextTag,0, "");
	}
	if(err!=noErr) msg->warning("Can't set selectedSong text (%d)!!",err);
	else { lock(); plDisplay=row; unlock(); }
}

void CarbonChannel::plMove(int from, int dest) {
	lock();
	int start = from;
	while(start != dest) {
		if(start < dest) { /* move down */
			playList->movedown(start);
			start++;
		}
		else if(start > dest) {
			playList->moveup(start);
			start--;
		}
	}
	unlock();
	plUpdate();
}

void CarbonChannel::plRemove(int pos) {
	lock();
	if(playList->selected_pos()==pos) {
		if(playList->len() > 1) {
			if(inChannel->state==1.0) inChannel->skip();
			else {
				if(pos==playList->len()) {
					plSelect(pos-1);
				}
				else {
					plSelect(pos+1);
				}
			}
		}
		else {
			inChannel->stop();
			plSelect(0);
		}
	}
	playList->rem(pos);
	unlock();
	plUpdate();
}

void CarbonChannel::plRemoveSelection() {
	DataBrowserItemID first,last;
	GetDataBrowserSelectionAnchor(playListControl,&first,&last);
	for (int i=first;i<=last;i++) {
		plRemove(first);
	}
}

bool CarbonChannel::checkNeighbours() {
	if(!isAttached) return parent->attract_channels(chIndex,&neigh);
	return false;
}

void CarbonChannel::attractNeighbour() {
	if(neigh.channel) {
		OpenDrawer(fader,neigh.position==ATTACH_RIGHT?kWindowEdgeRight:kWindowEdgeLeft,false);
		ActivateWindow(neigh.channel->window,true);
	}
}

void CarbonChannel::stopAttracting() {
	if(!isAttached) {
		CloseDrawer(fader,false);
		//SelectWindow(me->window);
		if(neigh.channel) {
			ActivateWindow(neigh.channel->window,false);
			memset(&neigh,0,sizeof(neigh));
		//	isAttached=false;
		//	isSlave=false;
		}
	}
}

void CarbonChannel::stopFading() {
	if(isAttached) {
		if(isSlave) { /* slave channel */
			SetWindowGroup(window,faderGroup);
		}
		else { /* main channel */
			if(neigh.channel) {
				Rect myBounds;
				OSStatus err = GetWindowBounds(window,kWindowContentRgn,&myBounds);
				neigh.channel->stopFading();
				SInt32 offset=neigh.position==ATTACH_LEFT?20:-20;
				myBounds.left+=offset;
			//	SetWindowBounds(window,kWindowContentRgn,&myBounds);
				MoveWindow(window,myBounds.left+offset,myBounds.top-20,false);
				memset(&neigh,0,sizeof(neigh));
			}
			//ChangeWindowGroupAttributes(faderGroup,0,WINDOW_GROUP_ATTRIBUTES);
			CloseDrawer(fader,false);
			SetControlValue(faderControl,0);
		}
		isAttached=false;
		isSlave=false;
	}
}

void CarbonChannel::doAttach() {
	const ControlID faderChan1ID = { CARBON_GUI_APP_SIGNATURE, FADER_CHAN1_ID};
	const ControlID faderChan2ID = { CARBON_GUI_APP_SIGNATURE, FADER_CHAN2_ID};
	OSStatus err=noErr;
	if(neigh.channel && !isAttached) {
		neigh.channel->gotAttached(this);
		isAttached=true;
	//	err=ChangeWindowGroupAttributes(faderGroup,WINDOW_GROUP_ATTRIBUTES,0);
				
		err=SetWindowGroup(neigh.channel->window,faderGroup);
		if(err!=noErr) msg->warning("Can't add slave channel to window group (%d)!!",err);
		
		crossFade(0); /* reset volume for channel members */
		ControlRef textControl;
		err=GetControlByID(fader,&faderChan1ID,&textControl);
		if(err != noErr) {
		msg->error("Can't get faderChannel control ref (%d)!!",err);}
		
		/* label master channel */
		CFStringRef format = CFStringCreateWithCString(NULL,"Channel %d",0);
		CFStringRef wName = CFStringCreateWithFormat(NULL,NULL,format,chIndex);
		err=SetControlData (textControl, 0, kControlStaticTextCFStringTag,sizeof(CFStringRef), &wName);
		CFRelease(wName);
		err=GetControlByID(fader,&faderChan2ID,&textControl);
		
		/* label slave channel */
		wName = CFStringCreateWithFormat(NULL,NULL,format,neigh.channel->chIndex);
		err=SetControlData (textControl, 0, kControlStaticTextCFStringTag,sizeof(CFStringRef), &wName);
		CFRelease(wName);
		CFRelease(format);
		SetControlAction(faderControl,NewControlActionUPP(&faderHandler));
		redrawFader();
	}
}

void CarbonChannel::crossFade(int fadeVal) {
	ControlRef volCtrl;
	OSStatus err;
	/* XXX - jmix->crossfade() doesn't work properly?? :O */
	//jmix->crossfade(chIndex,neigh.channel->chIndex,(float)(100-fadeVal)/100,(float)fadeVal/100);
	setVol(100-fadeVal);
	neigh.channel->setVol(fadeVal);
	err = GetControlByID(window,&volId,&volCtrl);
	SetControlValue(volCtrl,100-fadeVal);
	err = GetControlByID(neigh.channel->window,&volId,&volCtrl);
	SetControlValue(volCtrl,fadeVal);
}

void CarbonChannel::setVol(int vol) {
	ControlRef volCtrl;
	OSStatus err = GetControlByID(window,&volId,&volCtrl);
	SetControlValue(volCtrl,(int)(vol));
	jmix->set_volume(chIndex,(float)vol/100);
}

void CarbonChannel::redrawFader() {
	OSStatus err;
	Rect myBounds;
	Rect neighBounds;
	if(!isDrawing) {
	isDrawing=true;
	err = GetWindowBounds(window,kWindowContentRgn,&myBounds);
	err = GetWindowBounds(neigh.channel->window,kWindowContentRgn,&neighBounds);
	SInt32 width = neighBounds.right-neighBounds.left;
	if(neigh.channel && isAttached) {
		ChangeWindowGroupAttributes(!isSlave?faderGroup:neigh.channel->faderGroup,0,WINDOW_GROUP_ATTRIBUTES);
		SInt32 edge = (neigh.position==ATTACH_LEFT)?(myBounds.left-140-width):(myBounds.right+140);
		SInt32 offset=edge-neighBounds.left;
		MoveWindow(neigh.channel->window,edge,myBounds.top,false);
		if(isSlave) {
			Rect faderBounds;
			err = GetWindowBounds(neigh.channel->fader,kWindowContentRgn,&faderBounds);
			MoveWindow(neigh.channel->fader,faderBounds.left+offset,faderBounds.top,false);
		}
		BringToFront(window);
		BringToFront(neigh.channel->window);
		ChangeWindowGroupAttributes(!isSlave?faderGroup:neigh.channel->faderGroup,WINDOW_GROUP_ATTRIBUTES,0);
	}
	isDrawing=false;
	}
}

void CarbonChannel::gotAttached(CarbonChannel *channel) {
	Rect neighBounds,myBounds;
	if(!channel) return;
	GetWindowBounds(channel->window,kWindowContentRgn,&neighBounds);
	GetWindowBounds(window,kWindowContentRgn,&myBounds);
	isAttached = true;
	neigh.channel = channel;
	neigh.position = (myBounds.left < neighBounds.left)?ATTACH_RIGHT:ATTACH_LEFT;
	isSlave = true;
}

bool CarbonChannel::attached() {
	return isAttached;
}

void CarbonChannel::startResize() {
	isResizing=true;
}

void CarbonChannel::stopResize() {
	isResizing=false;
}

bool CarbonChannel::resizing() {
	return isResizing;
}

bool CarbonChannel::slave() {
	return isSlave;
}

void CarbonChannel::openFileDialog() {
	OSStatus err = OpenFileWindow(window);
}

void CarbonChannel::openUrlDialog() {
	const ControlID openUrlTextID = { CARBON_GUI_APP_SIGNATURE, OPEN_URL_TEXT_CONTROL };
	ControlRef urlText;
	if(!IsWindowVisible(openUrl)) {
		ShowSheetWindow(openUrl,window);
	}
	else {
		BringToFront(openUrl);
	}
	OSStatus err=GetControlByID(openUrl,&openUrlTextID,&urlText);
	if(err!=noErr) msg->warning("Can't get text control from the openUrl dialog (%d)!!",err);
	//SelectWindow(openUrl);
	HIViewAdvanceFocus(HIViewGetRoot(openUrl),0); /* set focus to the url input text box */
}

void CarbonChannel::tryOpenUrl() {
	char *text;
	const ControlID openUrlTextID = { CARBON_GUI_APP_SIGNATURE, OPEN_URL_TEXT_CONTROL };
	ControlRef urlText;
	Size textSize;
	OSStatus err=GetControlByID(openUrl,&openUrlTextID,&urlText);
	if(err!=noErr) msg->warning("Can't get text control from the openUrl dialog (%d)!!",err);
	err= GetControlDataSize(urlText,0,kControlEditTextTextTag,&textSize);
	if(err!=noErr) msg->warning("Can't get url size (%d)!!",err);
	text=(char *)malloc(textSize+1);
	err=GetControlData(urlText,0,kControlEditTextTextTag,textSize,text,NULL);
	text[textSize]=0;
	if(!jmix->add_to_playlist(chIndex,text)) {
		msg->warning("Can't open url %s. Check syntax ",text);
	}
	else {
		SetControlData(urlText,0,kControlEditTextTextTag,0,NULL);
		HideSheetWindow(openUrl);
	}
	free(text);
}

void CarbonChannel::cancelOpenUrl() {
	ControlRef urlText;
	const ControlID openUrlTextID = { CARBON_GUI_APP_SIGNATURE, OPEN_URL_TEXT_CONTROL };
	OSStatus err=GetControlByID(openUrl,&openUrlTextID,&urlText);
	if(err!=noErr) msg->warning("Can't get text control from the openUrl dialog (%d)!!",err);
	SetControlData(urlText,0,kControlEditTextTextTag,0,NULL);
	HideSheetWindow(openUrl);
}

void CarbonChannel::run() { /* channel main loop */
	lock();
	/* update status - this can override the status setted by user controls
	 * in respect of inChannel behaviour */
	if(inChannel->state==0.0) 
		if(status!=CC_PAUSE) status=CC_STOP;
	else if(inChannel->state >= 2.0) status=CC_STOP;
	else status=CC_PLAY;
		
	if(status!=savedStatus) { /* status change */
		ControlID butId = { CARBON_GUI_APP_SIGNATURE ,PLAY_BUT};
		ControlRef playButton,pauseButton,stopButton;
		OSStatus err =  GetControlByID(window,&butId,&playButton);
		if(err!=noErr) msg->error("Can't get playButton control (%d)!!",err);
		butId.id=PAUSE_BUT;
		err=GetControlByID(window,&butId,&pauseButton);
		if(err!=noErr) msg->error("Can't get pauseButton control (%d)!!",err);
		butId.id=STOP_BUT;
		err=GetControlByID(window,&butId,&stopButton);
		if(err!=noErr) msg->error("Can't get pauseButton control (%d)!!",err);
		switch(status) {
			case CC_PLAY:
				DisableControl(playButton);
				EnableControl(stopButton);
				EnableControl(pauseButton);
				//SetControlValue(stopButton,0);
				//SetControlValue(pauseButton,0);
				break;
			case CC_STOP:
				EnableControl(playButton);
				DisableControl(stopButton);
				DisableControl(pauseButton);
				break;
			case CC_PAUSE:
				EnableControl(playButton);
				EnableControl(stopButton);
				DisableControl(pauseButton);
				break;
		}
		
		savedStatus=status;
	}
	unlock();
	if(plDisplay!=playList->selected_pos()) { /* should mantain lock until comparison? */
		updateSelectedSong(playList->selected_pos());
	}
	
}

void CarbonChannel::play() {
	if(!playList->selected_pos()) plSelect(1);
	if(jmix->play_channel(chIndex)) {
		func("Playing channel %d",chIndex);
	}
	else {
		msg->warning("Can't play channel %d!!",chIndex);
		func("Error trying to play channel %d!!",chIndex);
	}
	lock();
	status=CC_PLAY;
	unlock();
}

void CarbonChannel::stop() {
	if(jmix->stop_channel(chIndex)) {
		func("Channel %d stopped",chIndex);
		setPos(0);
		setLCD("00:00:00");
	}
	else {
		msg->warning("Can't stop channel %d!!",chIndex);
		func("Error trying to stop channel %d!!",chIndex);
	}
	lock();
	status=CC_STOP;
	unlock();
}

void CarbonChannel::prev() {
	int curPos;
	if(status==CC_PLAY) inChannel->stop();
	curPos = inChannel->playlist->selected_pos();
	plSelect((curPos>1)?((seek==0)?curPos-1:curPos):1);
	if(status==CC_PLAY) inChannel->play();
}

void CarbonChannel::next() {
	if(status==CC_PLAY) inChannel->stop();
	inChannel->skip();
	plSelect(inChannel->playlist->selected_pos());
	if(status==CC_PLAY) inChannel->play();
}

void CarbonChannel::pause() {
	jmix->pause_channel(chIndex);
	lock();
	status=CC_PAUSE;
	unlock();
}

/* End of CarbonChannel */

// --------------------------------------------------------------------------------------------------------------

/****************************************************************************/
/* LOOP TIMER */
/****************************************************************************/

void channelLoop(EventLoopTimerRef inTimer,void *inUserData) {
	/*OSStatus err;*/
}

// --------------------------------------------------------------------------------------------------------------

/****************************************************************************/
/* CALLBACKS */
/****************************************************************************/


OSErr ForceDrag (Point *mouse,SInt16 *modifiers,void *userData,DragRef theDrag) 
{
	*modifiers = 256|cmdKeyBit;
	return noErr;
}


Boolean AddDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item,DragItemRef *itemRef)
{
	DragItemRef refID = item;
	CarbonChannel *senderChannel;
	OSStatus err;
	err = GetControlProperty(browser,CARBON_GUI_APP_SIGNATURE,PLAYLIST_PROPERTY,sizeof(CarbonChannel *),NULL,&senderChannel);
	if(err!=noErr) { 
	//	senderChannel->msg->warning("Can't get the CarbonChannel object associated to the playList control (%d)!!",err);
		return false;
	}
	err = AddDragItemFlavor(theDrag,refID,PLAYLIST_ITEM_DRAG_ID,&item,sizeof(DataBrowserItemID),flavorSenderOnly);
	if(err!=noErr) {
		senderChannel->msg->warning("Can't add itemID falvour to new drag (%d)!!",err);
		return false;
	}
	err = AddDragItemFlavor(theDrag,refID,PLAYLIST_SENDER_DRAG_ID,&senderChannel,sizeof(CarbonChannel),flavorSenderOnly);
	if(err!=noErr) {
		senderChannel->msg->warning("Can't start drag (%d)!!",err);
		return false;
	}
	*itemRef = refID;
	return true;
}

OSStatus HandlePlaylist (ControlRef browser,DataBrowserItemID itemID, 
	DataBrowserPropertyID property,DataBrowserItemDataRef itemData, 
	Boolean changeValue)
{  
    OSStatus status = noErr;
	Url *entry;
	DataBrowserItemState state;
	OSStatus err;
	CarbonChannel *me;
	err = GetControlProperty(browser,CARBON_GUI_APP_SIGNATURE,PLAYLIST_PROPERTY,sizeof(CarbonChannel *),NULL,&me);
	if(err!=noErr) { 
		return err;
	}
	if (!changeValue) switch (property) 
    {
        case 'SONG':
			entry = (Url *)me->playList->pick(itemID);
			if(entry) {
				/* XXX - lazy coding */
				char *p = entry->path+strlen(entry->path);
				while(*p!='/') {
					if(p==entry->path) break;
					p--;
				}
				if(*p=='/') p++;
				status = SetDataBrowserItemDataText(itemData,
					CFStringCreateWithCString(kCFAllocatorDefault,
					p,kCFStringEncodingMacRoman));
				/*if(itemID==1 && me->playList->len()==1) { // First entry
					me->plSelect(1);
				}*/
			}
			break;
		default:
            status = errDataBrowserPropertyNotSupported;
            break;
    }
    else status = errDataBrowserPropertyNotSupported; 
 	return status;
}

Boolean HandleDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item) {
	DragItemRef dragItem;
	HFSFlavor draggedData;
	FlavorType receivedType;
	Size dataSize;
	DataBrowserItemID movedItem;
	DataBrowserItemState itemState = 0L;
	OSErr err;
	int targetPos = item;
	int itemsNum = 1;
	CarbonChannel *me;
	char *fileName;	
	err = GetControlProperty(browser,CARBON_GUI_APP_SIGNATURE,PLAYLIST_PROPERTY,sizeof(CarbonChannel *),NULL,&me);
	if(err!=noErr) return false;
	int removed = 0;
	while(GetDragItemReferenceNumber(theDrag,itemsNum,&dragItem) == noErr) itemsNum++;
	for(itemsNum--;itemsNum>0;itemsNum--) {
	    err = GetDragItemReferenceNumber(theDrag,itemsNum,&dragItem);
		if(err != noErr) {
			me->msg->warning("Can't get dragItem reference number at index %d (%d)",itemsNum,err);
			break;
		}
		err = GetFlavorType(theDrag,dragItem,1,&receivedType);
		if(err!=noErr) me->msg->error("Can't get type of the received drag (%d)!!",err);
		err = GetFlavorDataSize (theDrag,dragItem,receivedType,&dataSize);
		if(receivedType==kDragFlavorTypeHFS) { /* here for backward compatibility (prior to tiger */
			err = GetFlavorData(theDrag,dragItem,receivedType,&draggedData,&dataSize,0);
			FSRef fRef;
			err = FSpMakeFSRef (&draggedData.fileSpec,&fRef);
			fileName=(char *)malloc(kHFSPlusMaxFileNameChars);
			err = FSRefMakePath(&fRef,(UInt8 *)fileName,kHFSPlusMaxFileNameChars);
			if(!me->plAdd(fileName)) {
				me->msg->warning("Can't add %s to playList",fileName);
			}
			free(fileName);
		}
		else if(receivedType==typeFileURL) {
			CFURLRef url;
			CFStringRef path;
			char *buffer=(char *)malloc(dataSize);
			err = GetFlavorData( theDrag, dragItem, receivedType, buffer, &dataSize, 0 );
			if(err!=noErr) me->msg->warning("Can't get url for dropped object (%d)!!",err);
			url = CFURLCreateWithBytes( kCFAllocatorDefault, (UInt8*) buffer, dataSize, kCFStringEncodingUTF8, NULL );
			if(url!=NULL) {
				path = CFURLCopyFileSystemPath(url,kCFURLPOSIXPathStyle);
				if(path!=NULL) {
					fileName=(char *)malloc(CFStringGetLength(path)+1);
					if(CFStringGetCString(path,fileName,CFStringGetLength(path)+1,0)) {
						if(!me->plAdd(fileName)) {
							me->msg->warning("Can't add %s to playList",fileName);
						}
					}
					free(fileName);
					CFRelease(path);
				}
				CFRelease(url);
			}
			free(buffer);
		}
		else if(receivedType==PLAYLIST_ITEM_DRAG_ID) {
			CarbonChannel *sender;
			err = GetFlavorData(theDrag,dragItem,receivedType,&movedItem,&dataSize,0);
			movedItem-=removed;
			err = GetFlavorType(theDrag,dragItem,2,&receivedType);
			if(err!=noErr) me->msg->error("Can't get type of the received drag (%d)!!",err);
			err = GetFlavorData(theDrag,dragItem,receivedType,&sender,&dataSize,0);
			if(sender != me) { /* receiving a drag from another channel window */
				Url *entry = (Url *)sender->playList->pick(movedItem);
				if(entry) {
				if (item == kDataBrowserNoItem) {
					me->playList->addurl(entry->path);
					entry->rem();
					removed++;
				}
				else {
					me->playList->insert(entry,targetPos);
					removed++;
				}
				me->plUpdate();
				sender->plUpdate();
				}
			}
			else { /* internal drag ... just moving songs around inside our playlist */
				if (item == kDataBrowserNoItem) {
					me->plMove(movedItem,me->playList->len());
				}
				else {
					me->plMove(movedItem,targetPos);
				}
			}
			targetPos++;
		}
		else {
			return false;
		}
	}
	return true;
}

Boolean CheckDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item) {
	DragItemRef dragItem;
	FlavorType receivedType;
	Size dataSize;
	CarbonChannel *me;
	OSErr err;
	err = GetControlProperty(browser,CARBON_GUI_APP_SIGNATURE,PLAYLIST_PROPERTY,sizeof(CarbonChannel *),NULL,&me);
	if(err!=noErr) return false;
	err = GetDragItemReferenceNumber(theDrag,1,&dragItem);
	if(err != noErr) me->msg->error("Can't get dragItem reference number (%d)",err);
	
	err = GetFlavorType(theDrag,dragItem,1,&receivedType);
	if(err!=noErr) me->msg->error("Can't get type of the received drag (%d)!!",err);
	err = GetFlavorDataSize (theDrag,dragItem,receivedType,&dataSize);
	if(receivedType==kDragFlavorTypeHFS||typeFileURL)
		return true;
	else if(receivedType==PLAYLIST_ITEM_DRAG_ID) {
		if(item == kDataBrowserNoItem) {
			err = SetDragInputProc(theDrag,NewDragInputUPP(&ForceDrag),NULL);
			if(err!=noErr) me->msg->error("Can't set input proc for internal item(%d)!!",err);
		}
		return true;
	}
	
	return false;
}

void HandleNotification (ControlRef browser,DataBrowserItemID itemID,
   DataBrowserItemNotification message) 
{		
	DataBrowserItemState state;
    Url *entry;
	OSStatus err;
	CarbonChannel *me;
	err = GetControlProperty(browser,CARBON_GUI_APP_SIGNATURE,PLAYLIST_PROPERTY,sizeof(CarbonChannel *),NULL,&me);
	if(err!=noErr) return;
	switch (message) {
		case kDataBrowserItemDoubleClicked:
			entry = (Url *)me->playList->pick(itemID);
			GetDataBrowserItemState(browser,itemID,&state);
			if(state == kDataBrowserItemIsSelected) {
				if(me->playList->selected_pos() != itemID) {
					ControlRef textControl;
					err = GetControlByID(me->window,&selectedSongId,&textControl);
					if(err != noErr) {
						me->msg->error("Can't get selectedSong control ref (%d)!!",err);
					}
					me->plSelect(itemID);
				}
			}
			break;	
	}
}

void GetPLMenu (ControlRef browser,MenuRef *menu,UInt32 *helpType,
	CFStringRef *helpItemString, AEDesc *selection) 
{
	CarbonChannel *me;
	OSStatus err = GetControlProperty(browser,CARBON_GUI_APP_SIGNATURE,
		PLAYLIST_PROPERTY,sizeof(CarbonChannel *),NULL,&me);
	if(err!=noErr) return;
	*menu = me->plGetMenu();
}

/*
void SelectPLMenu (ControlRef browser,MenuRef menu,UInt32 selectionType,SInt16 menuID,MenuItemIndex menuItem) {

}
*/

void faderHandler (ControlRef theControl, ControlPartCode partCode) {
	CarbonChannel *me;
	OSStatus err = GetControlProperty(theControl,CARBON_GUI_APP_SIGNATURE,
		FADER_PROPERTY,sizeof(CarbonChannel *),NULL,&me);
	if(err==noErr) {
		me->crossFade(GetControlValue(theControl));
	}
}


// --------------------------------------------------------------------------------------------------------------

/****************************************************************************/
/* EVENT HANDLERS */
/****************************************************************************/

static OSStatus ChannelEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    OSStatus err = noErr;
    CarbonChannel *me = (CarbonChannel *)userData;
	switch (GetEventKind (event))
    {
		case kAEOpenDocuments:
			OpenFile(event,me);
			break;
        case kEventWindowClosed: 
            me->close();
            break;
		case kEventWindowActivated:
			me->activateMenuBar();
			break;
		case kEventWindowBoundsChanging:
			Rect myBounds;
			err = GetWindowBounds(me->window,kWindowContentRgn,&myBounds);
			if(myBounds.right-myBounds.left < CHANNEL_WINDOW_WIDTH_MIN ||
				myBounds.bottom-myBounds.top < CHANNEL_WINDOW_HEIGHT_MIN)
				{
					Rect newBounds;
					newBounds.top = myBounds.top;
					newBounds.left = myBounds.left;
					newBounds.right = newBounds.left+CHANNEL_WINDOW_WIDTH_MIN;
					newBounds.bottom = newBounds.top+CHANNEL_WINDOW_HEIGHT_MIN;
					
					//err = SetWindowBounds(me->window,kWindowContentRgn,&newBounds);
					//if(err!=noErr) me->msg->warning("%d",err);
					return noErr;
				}
			if(me->attached() && me->resizing()) {
				//if(me->slave() && me->neigh.position==ATTACH_RIGHT)
				//	me->neigh.channel->redrawFader();
				//else 
				me->redrawFader();
			}
			else {
				AttractedChannel *neigh;
				if(me->checkNeighbours()) me->attractNeighbour();
				else me->stopAttracting();
			}
			break;
		case kEventWindowBoundsChanged:
		case kEventWindowDragCompleted:
			me->doAttach();
			break;
		case kEventWindowResizeStarted:
			me->startResize();
			break;
		case kEventWindowResizeCompleted:
			me->stopResize();
			if(me->attached()) me->redrawFader();
			break;
		default:
            //activeChannel = me;
            break;
    }
    
    return CallNextEventHandler(nextHandler,event);

}

static OSStatus dataBrowserEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    OSStatus err = noErr;
    CarbonChannel *me = (CarbonChannel *)userData;
	switch (GetEventKind (event))
    {
		default:
            //activeChannel = me;
            break;
    }
    
    return CallNextEventHandler(nextHandler,event);

}

// --------------------------------------------------------------------------------------------------------------

/****************************************************************************/
/* COMMAND HANDLER */
/****************************************************************************/

static OSStatus channelCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    HICommand command; 
    OSStatus err = noErr;
	SInt16 val;
	int i;
    CarbonChannel *me = (CarbonChannel *)userData;
	err = GetEventParameter (event, kEventParamDirectObject,
        typeHICommand, NULL, sizeof(HICommand), NULL, &command);
    if(err != noErr) me->msg->error("Can't get event parameter!!");
	switch (command.commandID)
    {
        case PLAY_CMD:
			me->play();
			break;
		case STOP_CMD:
			me->stop();
			break;
		case PAUSE_CMD:
			me->pause();
			break;
		case NEXT_CMD:
			me->next();
			break;
		case PREV_CMD:
			me->prev();
			break;
		case VOL_CMD:
			ControlRef volCtrl;
			err = GetControlByID(me->window,&volId,&volCtrl);
			if(err != noErr) {
				me->msg->warning("Can't get volume control (%d)!!",err);
			}
			else {
				SInt16 vol = GetControlValue(volCtrl);
				func("Setting volume to %f for channel %d",((float)vol)/100,me->chIndex);
				me->jmix->set_volume(me->chIndex,((float)vol)/100); 
			}
			break;
		case MENU_REMOVE_CMD:
			me->plRemoveSelection();
			break;
		case OPEN_FILE_CMD:
			me->openFileDialog();
			break;
		case OPEN_URL_CMD:
			me->openUrlDialog();
			break;
		case NEWC_CMD:
			me->parent->new_channel();
			break;
		case SHOW_STREAMS_CMD:
			me->parent->showStreamWindow();
			break;
		default:
            err = eventNotHandledErr;
            break;
	}
	return err;
}

static OSStatus faderCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    HICommand command; 
    OSStatus err = noErr;
	SInt16 val;
	int i,curPos;
    CarbonChannel *me = (CarbonChannel *)userData;
	err = GetEventParameter (event, kEventParamDirectObject,
        typeHICommand, NULL, sizeof(HICommand), NULL, &command);
    if(err != noErr) me->msg->error("Can't get event parameter!!");
	switch (command.commandID)
    {
		case FADER_CLOSE_CMD:
			me->stopFading();
			break;
		case FADER_CMD:
			me->crossFade(GetControlValue(me->faderControl));
		break;
		default:
			err = eventNotHandledErr;
	}
	return err;
}

static OSStatus openUrlCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    HICommand command; 
    OSStatus err = noErr;
	SInt16 val;
    CarbonChannel *me = (CarbonChannel *)userData;
	err = GetEventParameter (event, kEventParamDirectObject,
        typeHICommand, NULL, sizeof(HICommand), NULL, &command);
    if(err != noErr) me->msg->error("Can't get event parameter!!");
	switch (command.commandID)
    {
		case OPEN_URL_BUTTON_CMD:
			me->tryOpenUrl();
			break;
		case CANCEL_CMD:
			me->cancelOpenUrl();
		default:
			err = eventNotHandledErr;
	}
	return err;
}


static  OSErr OpenFile(EventRef event,CarbonChannel *me)
{

	OSStatus		anErr;

	AEDescList	docList;				// list of docs passed in
	long		index, itemsInList;
	Boolean		wasAlreadyOpen;

	anErr = GetEventParameter( event, OPEN_DOCUMENT_DIALOG_PARAM, typeAEList,NULL,sizeof(AEDescList),NULL, &docList);
	//nrequire(anErr, GetFileList);

	anErr = AECountItems( &docList, &itemsInList);			// how many files passed in
//	nrequire(anErr, CountDocs);
	for (index = itemsInList; index > 0; index--)			// handle each file passed in
	{	
		AEKeyword	keywd;
		DescType	returnedType;
		Size		actualSize;
		FSRef 		fileRef;
		FSCatalogInfo	theCatInfo;
		
		anErr = AEGetNthPtr( &docList, index, typeFSRef, &keywd, &returnedType,
						(Ptr)(&fileRef), sizeof( fileRef ), &actualSize );
		//nrequire(anErr, AEGetNthPtr);

		anErr = FSGetCatalogInfo( &fileRef, kFSCatInfoFinderInfo, &theCatInfo, NULL, NULL, NULL );
		//nrequire(anErr, FSGetCatalogInfo);

		if (anErr == noErr) {
			char path[2048]; /* XXX - hardcoded max filename size */
			FSRefMakePath (&fileRef,(UInt8 *)path,2048);
			if(!me->plAdd(path)) {
				me->msg->warning("Can't add %s to playList",path);
			}
		}
	}

	return anErr;
} // OpenDocument
