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

/* local prototype for Carbon callbacks */


/* END of prototypes */

/****************************************************************************/
/* Globals */
/****************************************************************************/

const ControlID dataBrowserId = { CARBON_GUI_APP_SIGNATURE, PLAYLIST_BOX_ID };
const ControlID selectedSongId = { CARBON_GUI_APP_SIGNATURE, SELECTED_SONG_CONTROL };
const ControlID seekTimeId = { CARBON_GUI_APP_SIGNATURE, SEEK_TIME_CONTROL };
const ControlID seekId = { CARBON_GUI_APP_SIGNATURE, SEEK_CONTROL };
const ControlID volId = { CARBON_GUI_APP_SIGNATURE,VOLUME_CONTROL };

#define CARBON_CHANNEL_EVENTS 3
const EventTypeSpec windowEvents[] = {
        { kEventClassWindow, kEventWindowActivated },
		{ kEventClassWindow, kEventWindowGetClickActivation },
		{ kEventClassWindow, kEventWindowClosed },
		{ kEventClassWindow, kEventWindowActivated }
};
#define DATA_BROWSER_EVENTS 2
const EventTypeSpec dataBrowserEvents[] = {
		{kEventClassControl, kEventControlActivate},
		{kEventClassControl, kEventControlDragEnter},
	//	{kEventClassMouse,kEventMouseDown}
};

const EventTypeSpec channelCommands[] = {
	{ kEventClassCommand, kEventCommandProcess }
};
CarbonChannel *activeChannel = NULL;

/* Start of CarbonChannel */

CarbonChannel::CarbonChannel(Stream_mixer *mix,WindowRef mainWin,IBNibRef nib,unsigned int chan) {
	parent = mainWin;
	jmix = mix;
	nibRef = nib;
	chIndex = chan;
	OSStatus err;
	playList = new Playlist();
	jmix->set_playmode(chIndex,PLAYMODE_CONT);
	inChannel = jmix->chan[chIndex];
	
	memset(playListItems,0,sizeof(playListItems));
	
	msg = new CarbonMessage(nibRef);
	err = CreateWindowFromNib(nibRef, CFSTR("Channel"), &window);
	
	err = CreateMenuFromNib (nibRef,CFSTR("PLMenu"),&plMenu);
	if(err != noErr) {
		msg->error("Can't create plMenu ref (%d)!!",err);
	}
	err = CreateMenuFromNib (nibRef,CFSTR("PLEntryMenu"),&plEntryMenu);
	if(err != noErr) {
		msg->error("Can't create plMenu ref (%d)!!",err);
	}
	
	err = InstallEventHandler(GetWindowEventTarget(window),ChannelEventHandler,CARBON_CHANNEL_EVENTS,windowEvents,this,&windowEventHandler);
	if(err != noErr) { 
		msg->error("Can't install event handler for Channel control (%d)!!",err);
	}
	/* install tje channel command handler */
    err = InstallWindowEventHandler (window, 
            NewEventHandlerUPP (channelCommandHandler), 
            GetEventTypeCount(channelCommands), channelCommands, 
            this, NULL);
	if(err != noErr) msg->error("Can't install channel commandHandler");

	
	CFStringRef format = CFStringCreateWithCString(NULL,"Channel %d",0);
	CFStringRef wName = CFStringCreateWithFormat(NULL,NULL,format,chan);
	SetWindowTitleWithCFString (window,wName);
	
	err = GetControlByID(window,&dataBrowserId,&playListControl);
	if(err != noErr) {
		msg->error("Can't obtain dataBrowser ControlRef (%d)!!",err);
	}

	EventTargetRef dbTarget = GetControlEventTarget (playListControl);
	/* installs databrowser event handler */
	err = InstallEventHandler(dbTarget,dataBrowserEventHandler,DATA_BROWSER_EVENTS,dataBrowserEvents,this,&playListEventHandler);

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
		NewDataBrowserItemNotificationWithItemUPP(&HandleNotification);
	/* context menu handler */
	dbCallbacks.u.v1.getContextualMenuCallback=
		NewDataBrowserGetContextualMenuUPP(&getPLMenu);
	dbCallbacks.u.v1.selectContextualMenuCallback=
		NewDataBrowserSelectContextualMenuUPP(&selectPLMenu);
	/* drag starter */
	dbCallbacks.u.v1.addDragItemCallback=NewDataBrowserAddDragItemUPP(&AddDrag);
	
	/* custom callbacks */
//	DataBrowserCustomCallbacks  dbCustomCallbacks;
//	InitDataBrowserCustomCallbacks(&dbCustomCallbacks);
	/* Tracking Handler */
//	dbCustomCallbacks.u.v1.trackingCallback=NewDataBrowserTrackingUPP(&PlaylistTracking);	
	
	/* register callbacks */
	SetDataBrowserCallbacks(playListControl, &dbCallbacks); 
//	SetDataBrowserCustomCallbacks(playListControl,&dbCustomCallbacks);
    SetAutomaticControlDragTrackingEnabledForWindow (window, true);
	SetControlDragTrackingEnabled(playListControl,true);
	
	/* and finally we can show the channelwindow */
	ShowWindow(window);
	BringToFront(window);
	ControlRef volCtrl;
	err = GetControlByID(window,&volId,&volCtrl);
	SetControlValue(volCtrl,(int)(inChannel->volume*100));
	EventLoopTimerUPP timerUPP = NewEventLoopTimerUPP(channelLoop);
	err = InstallEventLoopTimer( GetCurrentEventLoop(),0,1,timerUPP,this,&updater);
	if(err!=noErr) msg->error("Can't install the idle eventloop handler(%d)!!",err);

}

CarbonChannel::~CarbonChannel() {
	/* remove the eventloop associated to our window */
	RemoveEventLoopTimer(updater);
	RemoveEventHandler(windowEventHandler);
	RemoveEventHandler(playListEventHandler);
	/* remove some local structures */
	delete playList;
	delete msg;
	/* TODO - maybe more cleaning is needed */
}

int CarbonChannel::getNextPlayListID() {
	int i;
	for (i=1;i<CARBON_MAX_PLAYLIST_ENTRIES;i++) {
		if(playListItems[i] == 0) 
			return i;
	}
	msg->warning("Too many entries in playlist...maximum is %d!!",CARBON_MAX_PLAYLIST_ENTRIES);
	return -1;
}

bool CarbonChannel::add_playlist(char *txt) {
//	lock();
	playList->addurl(txt);
	int idx = playList->len();
	int id = getNextPlayListID();
	if(id) {
		AddDataBrowserItems(playListControl,kDataBrowserNoItem,1,&id,kDataBrowserItemNoProperty);
		playListItems[id] = idx;
	}
//	unlock();
}

void CarbonChannel::close () {
	EventRef event;
	OSStatus err;
	err = CreateEvent (NULL,CARBON_GUI_EVENT_CLASS,CG_RMCH_EVENT,0,kEventAttributeUserEvent,&event);
	if(err != noErr) this->msg->error("Can't create rmCh event!!");
	SetEventParameter(event,CG_RMCH_EVENT_PARAM,typeCFIndex,sizeof(int),&this->chIndex);
	err = SendEventToEventTarget(event,GetWindowEventTarget(this->parent));
	if(err != noErr) {
		this->msg->error("Can't send rmCh event to mainWin!!");
	}
	//delete me;
}

MenuRef CarbonChannel::plGetMenu() {
	int i;
	for (i=1;i<=playList->length;i++) {
		if(IsDataBrowserItemSelected(playListControl,i)) {
			return plEntryMenu;
		}
	}
	return plMenu;
}

void CarbonChannel::setLCD(char *text) {
	EventRef updateEvent;
	OSStatus err;
	ControlRef lcdControl;
	if(strncmp(text,lcd,255) != 0) {
		strncpy(lcd,text,255);
		GetControlByID(window,&seekTimeId,&lcdControl);
		CFStringRef lcdText = CFStringCreateWithCString(NULL,text,kCFStringEncodingMacRoman);
		SetControlData (lcdControl, 0, kControlStaticTextCFStringTag,sizeof(CFStringRef), &lcdText);
	}
}

void CarbonChannel::setPos(int pos) {
	OSStatus err;
	ControlRef seekControl;
	if(pos!=seek) {
		err = GetControlByID(window,&seekId,&seekControl);
		if(err != noErr) msg->warning("Can't update seek control (%d)!!",err);
		SetControlValue(seekControl,pos);
		seek = pos;
	}
}

void CarbonChannel::plSelect(int row) {
	ControlRef textControl;
	OSStatus err;
	if(playList->selected_pos()!=row) {
		Url *entry = playList->pick(row);
		err = GetControlByID(activeChannel->window,&selectedSongId,&textControl);
		if(err != noErr) {
			activeChannel->msg->error("Can't get selectedSong control ref (%d)!!",err);
		}
		playList->sel(row);
		jmix->set_channel(chIndex,row);
		func("Selected playlist entry %d (%s)\n",row,entry->path);
		CFStringRef url = CFStringCreateWithCString(NULL,entry->path,kCFStringEncodingMacRoman);
		err=SetControlData (textControl, 0, kControlStaticTextCFStringTag,sizeof(CFStringRef), &url);
		if(err!=noErr) msg->warning("Can't set selectedSong text (%d)!!",err);
	}
}

void CarbonChannel::activateMenuBar() {
	OSStatus err;
	err = SetMenuBarFromNib(nibRef, CFSTR("PLMenu"));
	if(err != noErr) msg->error("Can't get MenuBar!!");
}

/* End of CarbonChannel */

/****************************************************************************/
/* LOOP TIMER */
/****************************************************************************/

void channelLoop(EventLoopTimerRef inTimer,void *inUserData) {
	/*OSStatus err;
	CarbonChannel *me = (CarbonChannel *)inUserData;
	if(IsWindowVisible(me->window)) {
		RgnHandle region;
		err = GetWindowRegion(me->window,kWindowContentRgn,region);
		if(err!=noErr) me->msg->error("Can't obtain window region (%4)!!",err);
		UpdateControls(me->window,region);
	}*/
}

/****************************************************************************/
/* CALLBACKS */
/****************************************************************************/
/*
DataBrowserTrackingResult PlaylistTracking (
   ControlRef browser,
   DataBrowserItemID itemID,
   DataBrowserPropertyID property,
   const Rect *theRect,
   Point startPt,
   EventModifiers modifiers
)
{
	return kDataBrowserNothingHit;
}*/

Boolean AddDrag (
   ControlRef browser,
   DragRef theDrag,
   DataBrowserItemID item,
   DragItemRef *itemRef
)
{
	DragItemRef refID = item;
	OSStatus err = AddDragItemFlavor(theDrag,refID,CARBON_GUI_APP_SIGNATURE,&item,NULL,flavorSenderOnly);
	if(err!=noErr) activeChannel->msg->error("Can't start drag (%d)!!",err);
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
	if (!changeValue) switch (property) 
    {
        case 'SONG':
			entry = (Url *)activeChannel->playList->pick(activeChannel->playListItems[itemID]);
			status = SetDataBrowserItemDataText(itemData,
				CFStringCreateWithCString(kCFAllocatorDefault,
					entry->path,kCFStringEncodingMacRoman));
			if(activeChannel->playListItems[itemID]==1) { // First entry
				activeChannel->plSelect(1);
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
	OSErr err = GetDragItemReferenceNumber(theDrag,1,&dragItem);
	if(err != noErr) {
		printf("Can't get dragItem reference number (%d)",err);
	}
	err = GetFlavorType(theDrag,dragItem,1,&receivedType);
	if(err!=noErr) activeChannel->msg->error("Can't get type of the received drag (%d)!!",err);
	err = GetFlavorDataSize (theDrag,dragItem,receivedType,&dataSize);
	if(receivedType==kDragFlavorTypeHFS) {
//	if (item == kDataBrowserNoItem) {
//		itemState = kDataBrowserContainerIsOpen;
//		printf("EKKOMI\n");
//	}
		err = GetFlavorData(theDrag,dragItem,receivedType,&draggedData,&dataSize,0);
		FSRef fRef;
		err = FSpMakeFSRef (&draggedData.fileSpec,&fRef);
	
		char fileName[kHFSPlusMaxFileNameChars]; /* MMMM... if not HFSPLUS fileName is too large */
		err = FSRefMakePath(&fRef,(UInt8 *)fileName,kHFSPlusMaxFileNameChars);
		return activeChannel->jmix->add_to_playlist(activeChannel->chIndex,fileName);
	}
	else if(receivedType==CARBON_GUI_APP_SIGNATURE) {
		err = GetFlavorData(theDrag,dragItem,receivedType,&movedItem,&dataSize,0);
	
		return true;
	}
	else {
		printf("CIAO \n\n\n\n");
	}
	return false;
}

Boolean CheckDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item) {
	// DragItemRef dragItem;
	return true;
}

void HandleNotification (ControlRef browser,DataBrowserItemID itemID,
   DataBrowserItemNotification message,DataBrowserItemDataRef itemData) 
{		
	DataBrowserItemState state;
    Url *entry;
	OSStatus err;
	switch (message) {
		case kDataBrowserItemDoubleClicked:
			int itemPos = activeChannel->playListItems[itemID];
			entry = (Url *)activeChannel->playList->pick(itemPos);
			GetDataBrowserItemState(browser,itemID,&state);
			if(state == kDataBrowserItemIsSelected) {
				if(activeChannel->playList->selected_pos() != itemPos) {
					ControlRef textControl;
					err = GetControlByID(activeChannel->window,&selectedSongId,&textControl);
					if(err != noErr) {
						activeChannel->msg->error("Can't get selectedSong control ref (%d)!!",err);
					}
					activeChannel->playList->sel(itemPos);
					activeChannel->jmix->set_channel(activeChannel->chIndex,itemPos);
					printf("Selected playlist entry %d (%s)\n",itemPos,entry->path);
					CFStringRef url = CFStringCreateWithCString(NULL,entry->path,kCFStringEncodingMacRoman);
					err=SetControlData (textControl, 0, kControlStaticTextCFStringTag,sizeof(CFStringRef), &url);
					if(err!=noErr) activeChannel->msg->warning("Can't set selectedSong text (%d)!!",err);
				}
			}
			break;	
	}
}

void getPLMenu (ControlRef browser,MenuRef *menu,UInt32 *helpType,CFStringRef *helpItemString, AEDesc *selection) {
	*menu = activeChannel->plGetMenu();
}

void selectPLMenu (ControlRef browser,MenuRef menu,UInt32 selectionType,SInt16 menuID,MenuItemIndex menuItem) {

}

void RemovePlaylistItem (
   DataBrowserItemID item,
   DataBrowserItemState state,
   void *clientData) 
{
	CarbonChannel *me = (CarbonChannel *)clientData;
	int i = me->playListItems[item];
	me->jmix->rem_from_playlist(me->chIndex,i);
	me->playList->rem(i);
	int n;
	me->playListItems[i]=0;
	for(n=1;n<CARBON_MAX_PLAYLIST_ENTRIES;n++) {
		if(me->playListItems[n] > i) me->playListItems[n]--;
	}
	RemoveDataBrowserItems(me->playListControl,kDataBrowserNoItem,1,&item,kDataBrowserItemNoProperty);
}


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
        case kEventWindowClosed: 
            me->close();
            break;
		case kEventWindowActivated:
			me->activateMenuBar();
		default:
            activeChannel = me;
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
            activeChannel = me;
            break;
    }
    
    return CallNextEventHandler(nextHandler,event);

}

/****************************************************************************/
/* COMMAND HANDLER */
/****************************************************************************/

static OSStatus channelCommandHandler (
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
        case PLAY_CMD:
			if(me->jmix->play_channel(me->chIndex)) {
				func("Playing channel %d",me->chIndex);
			}
			else {
				me->msg->warning("Can't play channel %d!!",me->chIndex);
				func("Error trying to play channel %d!!",me->chIndex);
			}
			break;
		case STOP_CMD:
			if(me->jmix->stop_channel(me->chIndex)) {
				func("Channel %d stopped",me->chIndex);
			}
			else {
				me->msg->warning("Can't stop channel %d!!",me->chIndex);
				func("Error trying to stop channel %d!!",me->chIndex);
			}
			break;
		case PAUSE_CMD:
			me->jmix->pause_channel(me->chIndex);
			break;
		case NEXT_CMD:
			me->inChannel->stop();
			me->inChannel->skip();
			me->plSelect(me->inChannel->playlist->selected_pos());
			me->inChannel->play();
			break;
		case PREV_CMD:
			me->inChannel->stop();
			curPos = me->inChannel->playlist->selected_pos();
			me->plSelect((curPos>1)?((me->seek==0)?curPos-1:curPos):1);
			me->inChannel->play();
		case VOL_CMD:
			ControlRef volCtrl;
			err = GetControlByID(me->window,&volId,&volCtrl);
			if(err != noErr) {
				me->msg->warning("Can't get volume control (%d)!!",err);
			}
			else {
				SInt16 vol = GetControlValue(volCtrl);
				/* XXX - mmmm i'm not sure that this is a good int to float conversion */
				func("Setting volume to %f for channel %d",((float)vol)/100,me->chIndex);
				me->jmix->set_volume(me->chIndex,((float)vol)/100); 
			}
			break;
		case MENU_REMOVE_CMD:
			ForEachDataBrowserItem(me->playListControl,kDataBrowserNoItem,true,kDataBrowserItemIsSelected,
				NewDataBrowserItemUPP(&RemovePlaylistItem), me);
			break;
		default:
            err = eventNotHandledErr;
            break;
	}
}
