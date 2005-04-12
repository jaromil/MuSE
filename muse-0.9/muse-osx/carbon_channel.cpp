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

/* Event handlers */
static OSStatus ChannelEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);

static OSStatus dataBrowserEventHandler(
	EventHandlerCallRef nextHandler, EventRef event, void *userData);
	
static OSStatus channelCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);
	
/* DataBrowser handlers */
OSStatus HandlePlaylist (ControlRef browser,DataBrowserItemID itemID,
	DataBrowserPropertyID property,DataBrowserItemDataRef itemData,Boolean changeValue);

Boolean HandleDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item);

Boolean CheckDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item);

void getPLMenu (ControlRef browser,MenuRef *menu,UInt32 *helpType,
	CFStringRef *helpItemString, AEDesc *selection);

void selectPLMenu(ControlRef browser,MenuRef menu,UInt32 selectionType,
	SInt16 menuID,MenuItemIndex menuItem);

/* END of prototypes */

/* Globals */

const ControlID dataBrowserId = { CARBON_GUI_APP_SIGNATURE, PLAYLIST_BOX_ID };
#define CARBON_CHANNEL_EVENTS 4
const EventTypeSpec windowEvents[] = {
        { kEventClassWindow, kEventWindowActivated },
		{ kEventClassWindow, kEventWindowGetClickActivation },
		{ kEventClassWindow, kEventWindowClosed },
};
#define DATA_BROWSER_EVENTS 1
const EventTypeSpec dataBrowserEvents[] = {
		{kEventClassControl, kEventControlDragEnter}
};

CarbonChannel *activeChannel = NULL;
	
/* Start of CarbonChannel */

CarbonChannel::CarbonChannel(Stream_mixer *mix,WindowRef mainWin,IBNibRef nib,unsigned int chan) {
	parent = mainWin;
	jmix = mix;
	nibRef = nib;
	chIndex = chan;
	OSStatus err;
	DataBrowserCallbacks  dbCallbacks;
	playList = new Playlist();
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
	
	err = InstallEventHandler(GetWindowEventTarget(window),ChannelEventHandler,CARBON_CHANNEL_EVENTS,windowEvents,this,NULL);
	if(err != noErr) { 
		msg->error("Can't install event handler for Channel control (%d)!!",err);
	}
	
	CFStringRef format = CFStringCreateWithCString(NULL,"Channel %d",0);
	CFStringRef wName = CFStringCreateWithFormat(NULL,NULL,format,chan);
	SetWindowTitleWithCFString (window,wName);
	
	err = GetControlByID(window,&dataBrowserId,&playListControl);
	if(err != noErr) {
		msg->error("Can't obtain dataBrowser ControlRef (%d)!!",err);
	}
	EventTargetRef dbTarget = GetControlEventTarget (playListControl);
	err = InstallEventHandler(dbTarget,dataBrowserEventHandler,DATA_BROWSER_EVENTS,dataBrowserEvents,this,NULL);
	
	/* installs databrowser callbacks */
	dbCallbacks.version = kDataBrowserLatestCallbacks; 
    InitDataBrowserCallbacks (&dbCallbacks); 

	/* main callback */
    dbCallbacks.u.v1.itemDataCallback=NewDataBrowserItemDataUPP(HandlePlaylist); 
	/* callback to check if we have to accept a drag */
	dbCallbacks.u.v1.acceptDragCallback=NewDataBrowserAcceptDragUPP(&CheckDrag);
	/* Drag handler */
	dbCallbacks.u.v1.receiveDragCallback=NewDataBrowserReceiveDragUPP(&HandleDrag);
    /* context menu handler */
	dbCallbacks.u.v1.getContextualMenuCallback=
		NewDataBrowserGetContextualMenuUPP(&getPLMenu);
	dbCallbacks.u.v1.selectContextualMenuCallback=
		NewDataBrowserSelectContextualMenuUPP(&selectPLMenu);

	/* register callbacks */
	SetDataBrowserCallbacks(playListControl, &dbCallbacks); 
    SetAutomaticControlDragTrackingEnabledForWindow (window, true);
	
	/* and finally we can show the channelwindow */
	ShowWindow(window);
	BringToFront(window);
}

CarbonChannel::~CarbonChannel() {
	delete playList;
	delete msg;
}

bool CarbonChannel::add_playlist(char *txt) {
	playList->addurl(txt);
	const DataBrowserItemID idx = playList->len();
	AddDataBrowserItems (playListControl,kDataBrowserNoItem,1,&idx,kDataBrowserItemNoProperty);
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

MenuRef CarbonChannel::get_pl_menu() {
	if(playList->selected_pos()) {
		return plEntryMenu;
	}
	else {
		return plMenu;
	}

}
/* End of CarbonChannel */

/* *** CALLBACKS *** */

OSStatus HandlePlaylist (ControlRef browser,DataBrowserItemID itemID, 
	DataBrowserPropertyID property,DataBrowserItemDataRef itemData, 
	Boolean changeValue)
{  
    OSStatus status = noErr;
	DataBrowserItemState state;
    Url *entry;
	if (!changeValue) switch (property) 
    {
        case 'SONG':
			entry = (Url *)activeChannel->playList->pick(itemID);
			status = SetDataBrowserItemDataText(itemData,
				CFStringCreateWithCString(kCFAllocatorDefault,
					entry->path,kCFStringEncodingMacRoman));
			break;
		case kDataBrowserItemIsActiveProperty:
			entry = (Url *)activeChannel->playList->pick(itemID);
			GetDataBrowserItemState(browser,itemID,&state);
			if(state == kDataBrowserItemIsSelected) {
				if(activeChannel->playList->selected_pos() != itemID) {
					activeChannel->playList->sel(itemID);
					activeChannel->jmix->set_channel(activeChannel->chIndex,itemID);
					printf("Selected playlist entry %d (%s)\n",property,entry->path);
				}
			}
			else {
				int selPos = activeChannel->playList->selected_pos();
				if(selPos == itemID) {
					entry = (Url *)activeChannel->playList->pick(activeChannel->playList->selected_pos());
					GetDataBrowserItemState(browser,activeChannel->playList->selected_pos(),&state);
					if(state != kDataBrowserItemIsSelected) {
						printf("Deselected playlist entry %d (%s)\n",selPos,entry->path);
						entry->sel(false);
					}
				}
			}
			break;
        default:
            status = errDataBrowserPropertyNotSupported;
            break;
    }
    else status = errDataBrowserPropertyNotSupported; 
 	return status;
}

Boolean HandleDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item){
	DragItemRef dragItem;
	HFSFlavor draggedData;
	OSErr err = GetDragItemReferenceNumber(theDrag,1,&dragItem);
	if(err != noErr) {
		printf("Can't get dragItem reference number (%d)",err);
	}
	Size dataSize;
	err = GetFlavorDataSize (theDrag,dragItem,kDragFlavorTypeHFS,&dataSize);
	err = GetFlavorData(theDrag,dragItem,kDragFlavorTypeHFS,&draggedData,&dataSize,0);
	FSRef fRef;
	err = FSpMakeFSRef (&draggedData.fileSpec,&fRef);
	
	char fileName[kHFSPlusMaxFileNameChars]; /* MMMM... if not HFSPLUS fileName is too large */
	err = FSRefMakePath(&fRef,(UInt8 *)fileName,kHFSPlusMaxFileNameChars);
	return activeChannel->jmix->add_to_playlist(activeChannel->chIndex,fileName);
}

Boolean CheckDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item) {
	// DragItemRef dragItem;
	return true;
}

void getPLMenu (ControlRef browser,MenuRef *menu,UInt32 *helpType,CFStringRef *helpItemString, AEDesc *selection) {
	*menu = activeChannel->get_pl_menu();
}

void selectPLMenu (ControlRef browser,MenuRef menu,UInt32 selectionType,SInt16 menuID,MenuItemIndex menuItem) {

}

/* EVENT HANDLERS */

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


static OSStatus channelCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
/*
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
			val = GetControlValue(me->mainControls[SNDOUT_BUT]);
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
		case 'stre':
		case 'sndi':
		case 'vol ':
		case 'abou':
        default:
            err = eventNotHandledErr;
            break;
*/
	}

