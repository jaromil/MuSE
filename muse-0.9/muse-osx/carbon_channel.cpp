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

const ControlID dataBrowserId = { CARBON_GUI_APP_SIGNATURE, PLAYLIST_BOX_ID };

/* local prototype for Carbon callbacks */
OSStatus close (EventHandlerCallRef nextHandler,EventRef inEvent, void *userData);
OSStatus HandlePlaylist (ControlRef browser,DataBrowserItemID itemID,
	DataBrowserPropertyID property,DataBrowserItemDataRef itemData,Boolean changeValue);
Boolean HandleDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item);
Boolean CheckDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item);
void getPLMenu (ControlRef browser,MenuRef *menu,UInt32 *helpType,CFStringRef *helpItemString, AEDesc *selection);
void selectPLMenu(ControlRef browser,MenuRef menu,UInt32 selectionType,SInt16 menuID,MenuItemIndex menuItem);
/* END of prototypes

/* Globals */
CarbonChannel *me = NULL;
	
/* Start of CarbonChannel */

CarbonChannel::CarbonChannel(Stream_mixer *mix,WindowRef mainWin,IBNibRef nib,unsigned int chan) {
	parent = mainWin;
	jmix = mix;
	nibRef = nib;
	chIndex = chan;
	OSStatus err;
	EventTypeSpec evtClose;
	EventTypeSpec evtClick;
	evtClose.eventClass = kEventClassWindow;
	evtClose.eventKind = kEventWindowClose;
	DataBrowserCallbacks  dbCallbacks;
	playList = new Playlist();
	msg = new CarbonMessage(nibRef);
	me = this;
	err = CreateWindowFromNib(nibRef, CFSTR("Channel"), &window);
	
	err = InstallEventHandler(GetWindowEventTarget(window),close,1,&evtClose,this,NULL);
	if(err != noErr) { 
		msg->error("Can't close event handler for Channel control (%d)!!",err);
	}
	
	CFStringRef format = CFStringCreateWithCString(NULL,"Channel %d",0);
	CFStringRef wName = CFStringCreateWithFormat(NULL,NULL,format,chan);
	SetWindowTitleWithCFString (window,wName);
	
	err = GetControlByID(window,&dataBrowserId,&playListControl);
	if(err != noErr) {
		msg->error("Can't obtain dataBrowser ControlRef (%d)!!",err);
	}
	
	err = CreateMenuFromNib (nibRef,CFSTR("PLMenu"),&plMenu);
	if(err != noErr) {
		msg->error("Can't create plMenu ref (%d)!!",err);
	}
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
	dbCallbacks.u.v1.getContextualMenuCallback=NewDataBrowserGetContextualMenuUPP(&getPLMenu);
	dbCallbacks.u.v1.selectContextualMenuCallback=NewDataBrowserSelectContextualMenuUPP(&selectPLMenu);
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

/* End of CarbonChannel */

/* *** CALLBACKS *** */
OSStatus close (EventHandlerCallRef nextHandler,EventRef inEvent, void *userData) {
	CarbonChannel *me = (CarbonChannel *)userData;
	EventRef event;
	OSStatus err;
	err = CreateEvent (NULL,CARBON_GUI_EVENT_CLASS,CG_RMCH_EVENT,0,kEventAttributeUserEvent,&event);
	if(err != noErr) me->msg->error("Can't create rmCh event!!");
	SetEventParameter(event,CG_RMCH_EVENT_PARAM,typeCFIndex,sizeof(int),&me->chIndex);
	err = SendEventToEventTarget(event,GetWindowEventTarget(me->parent));
	if(err != noErr) {
	}
	//delete me;
	return CallNextEventHandler (nextHandler, inEvent);
}

OSStatus    HandlePlaylist (ControlRef browser,
                DataBrowserItemID itemID, 
                DataBrowserPropertyID property, 
                DataBrowserItemDataRef itemData, 
                Boolean changeValue)
{  
    OSStatus status = noErr;
    Url *entry;
	if (!changeValue) switch (property) 
    {
        case 'SONG':
			entry = (Url *)me->playList->pick(itemID);
			status = SetDataBrowserItemDataText(itemData,
				CFStringCreateWithCString(kCFAllocatorDefault,entry->path,kCFStringEncodingMacRoman));
			break;
        default:
            status = errDataBrowserPropertyNotSupported;
            break;
    }
    else status = errDataBrowserPropertyNotSupported; 
// 3
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
	return me->jmix->add_to_playlist(me->chIndex,fileName);
}

Boolean CheckDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item) {
	// DragItemRef dragItem;
	return true;
}

void getPLMenu (ControlRef browser,MenuRef *menu,UInt32 *helpType,CFStringRef *helpItemString, AEDesc *selection) {
	*menu = me->plMenu;
}

void selectPLMenu (ControlRef browser,MenuRef menu,UInt32 selectionType,SInt16 menuID,MenuItemIndex menuItem) {

}

