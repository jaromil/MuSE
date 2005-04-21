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

/****************************************************************************/
/* Globals */
/****************************************************************************/

const ControlID dataBrowserId = { CARBON_GUI_APP_SIGNATURE, PLAYLIST_BOX_ID };
const ControlID selectedSongId = { CARBON_GUI_APP_SIGNATURE, SELECTED_SONG_CONTROL };
const ControlID seekTimeId = { CARBON_GUI_APP_SIGNATURE, SEEK_TIME_CONTROL };
const ControlID seekId = { CARBON_GUI_APP_SIGNATURE, SEEK_CONTROL };
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

const EventTypeSpec channelCommands[] = {
	{ kEventClassCommand, kEventCommandProcess }
};
const EventTypeSpec faderCommands[] = {
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
	jmix->set_playmode(chIndex,PLAYMODE_CONT);
	inChannel = jmix->chan[chIndex];
	memset(&neigh,0,sizeof(neigh));
	isAttached=false;
	isSlave=false;
	isResizing=false;
	isDrawing=false;
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
	/* install tje channel command handler */
    err = InstallWindowEventHandler (window, 
            NewEventHandlerUPP (channelCommandHandler), 
            GetEventTypeCount(channelCommands), channelCommands, 
            this, NULL);
	if(err != noErr) msg->error("Can't install channel commandHandler");

	
	CFStringRef format = CFStringCreateWithCString(NULL,"Channel %d",0);
	CFStringRef wName = CFStringCreateWithFormat(NULL,NULL,format,chan);
	SetWindowTitleWithCFString (window,wName);
	
	/* and finally we can show the channelwindow */
	ShowWindow(window);
	BringToFront(window);
	ControlRef volCtrl;
	err = GetControlByID(window,&volId,&volCtrl);
	SetControlValue(volCtrl,(int)(inChannel->volume*100));
	EventLoopTimerUPP timerUPP = NewEventLoopTimerUPP(channelLoop);
	err = InstallEventLoopTimer( GetCurrentEventLoop(),0,1,timerUPP,this,&updater);
	if(err!=noErr) msg->error("Can't install the idle eventloop handler(%d)!!",err);

	err = CreateWindowFromNib(nibRef,CFSTR("FaderWindow"),&fader);
	if(err!=noErr) msg->error("Can't create fader drawer (%d)!!",err);
	SetDrawerParent(fader,window);
	SetDrawerPreferredEdge(fader,kWindowEdgeRight);
	SetDrawerOffsets(fader,50,50);
	err = InstallWindowEventHandler (fader, 
            NewEventHandlerUPP (faderCommandHandler), 
            GetEventTypeCount(faderCommands), faderCommands, 
            this, NULL);
	err=CreateWindowGroup(0,&faderGroup);

	/* setup playList control */
	plSetup();

	SetAutomaticControlDragTrackingEnabledForWindow (window, true);
}

CarbonChannel::~CarbonChannel() {
	/* remove the eventloop associated to our window */
	AERemoveEventHandler (kCoreEventClass,kAEOpenDocuments,openHandler,false);
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
	/* TODO - maybe more cleaning is needed */
}


void CarbonChannel::plSetup() {
	OSStatus err;
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
		NewDataBrowserItemNotificationWithItemUPP(&HandleNotification);
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
	int i;
	int len = playList->len();
	DataBrowserItemID idList[len];
	RemoveDataBrowserItems(playListControl,kDataBrowserNoItem,0,NULL,kDataBrowserItemNoProperty);
	for(i=1;i<=len;i++) {
	Url *entry = playList->pick(i);
		idList[i-1] = i;
	}
	AddDataBrowserItems(playListControl,kDataBrowserNoItem,len,idList,kDataBrowserItemNoProperty);
}

bool CarbonChannel::plAdd(char *txt) {
	playList->addurl(txt);
	int id = playList->len();
	if(id) 
		AddDataBrowserItems(playListControl,kDataBrowserNoItem,1,&id,kDataBrowserItemNoProperty);
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
		playList->sel(row);
	}
	Url *entry = playList->pick(row);
	err = GetControlByID(window,&selectedSongId,&textControl);
	if(err != noErr) {
		msg->error("Can't get selectedSong control ref (%d)!!",err);
	}
	/* XXX - lazy coding */
	char *p = entry->path+strlen(entry->path);
	while(*p!='/') {
		if(p==entry->path) break;
		p--;
	}
	if(*p=='/') p++;
	func("Selected playlist entry %d (%s)\n",row,p);
	CFStringRef url = CFStringCreateWithCString(NULL,p,kCFStringEncodingMacRoman);
	err=SetControlData (textControl, 0, kControlStaticTextCFStringTag,sizeof(CFStringRef), &url);
	if(err!=noErr) msg->warning("Can't set selectedSong text (%d)!!",err);
}

void CarbonChannel::plMove(int from, int dest) {
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
	plUpdate();
}

void CarbonChannel::plRemove(int pos) {
	if(playList->selected_pos()==pos) {
		inChannel->skip();
	} 
	playList->rem(pos);
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
		if(!isSlave) {
			ChangeWindowGroupAttributes(faderGroup,0,WINDOW_GROUP_ATTRIBUTES);
			CloseDrawer(fader,false);
			Rect myBounds;
		OSStatus err = GetWindowBounds(window,kWindowContentRgn,&myBounds);
		if(err==noErr) {
			SInt32 offset=neigh.position==ATTACH_LEFT?20:-20;
			myBounds.left+=offset;
		//	SetWindowBounds(window,kWindowContentRgn,&myBounds);
			MoveWindow(window,myBounds.left+offset,myBounds.top-offset,false);
		}
			neigh.channel->stopFading();
			memset(&neigh,0,sizeof(neigh));
		}
		isAttached=false;
		isSlave=false;
	}
}

void CarbonChannel::tryAttach() {
	OSStatus err;
	if(neigh.channel && !isAttached) {
		neigh.channel->gotAttached(this);
		isAttached=true;
		err=ChangeWindowGroupAttributes(faderGroup,WINDOW_GROUP_ATTRIBUTES,0);
		if(err!=noErr) msg->warning("%d",err);
		err=SetWindowGroup(window,faderGroup);
		if(err!=noErr) msg->warning("%d",err);
		err=SetWindowGroup(neigh.channel->window,faderGroup);
		if(err!=noErr) msg->warning("%d",err);
		err=SetWindowGroup(fader,faderGroup);
		SetWindowGroupOwner(faderGroup,window);
		redrawFader();
	}
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
	if(!channel) return false;
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
				if(itemID==1) { // First entry
					me->plSelect(1);
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
		if(receivedType==kDragFlavorTypeHFS) {
			err = GetFlavorData(theDrag,dragItem,receivedType,&draggedData,&dataSize,0);
			FSRef fRef;
			err = FSpMakeFSRef (&draggedData.fileSpec,&fRef);
	
			char fileName[kHFSPlusMaxFileNameChars]; /* MMMM... if not HFSPLUS fileName is too large */
			err = FSRefMakePath(&fRef,(UInt8 *)fileName,kHFSPlusMaxFileNameChars);
			if(!me->jmix->add_to_playlist(me->chIndex,fileName)) {
				me->msg->warning("Can't add %s to playList",fileName);
			}
		}
		else if(receivedType==PLAYLIST_ITEM_DRAG_ID) {
			CarbonChannel *sender;
			err = GetFlavorData(theDrag,dragItem,receivedType,&movedItem,&dataSize,0);
			movedItem-=removed;
			err = GetFlavorType(theDrag,dragItem,2,&receivedType);
			if(err!=noErr) me->msg->error("Can't get type of the received drag (%d)!!",err);
			err = GetFlavorData(theDrag,dragItem,receivedType,&sender,&dataSize,0);
			if(sender != me) { /* receiving a drag from another channel window */
				Url *entry = sender->playList->pick(movedItem);
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
	if(receivedType==kDragFlavorTypeHFS)
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
   DataBrowserItemNotification message,DataBrowserItemDataRef itemData) 
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

void GetPLMenu (ControlRef browser,MenuRef *menu,UInt32 *helpType,CFStringRef *helpItemString, AEDesc *selection) {
	CarbonChannel *me;
	OSStatus err = GetControlProperty(browser,CARBON_GUI_APP_SIGNATURE,PLAYLIST_PROPERTY,sizeof(CarbonChannel *),NULL,&me);
	if(err!=noErr) return;
	*menu = me->plGetMenu();
}
/*
void SelectPLMenu (ControlRef browser,MenuRef menu,UInt32 selectionType,SInt16 menuID,MenuItemIndex menuItem) {

}
*/

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
					printf("CIAO %d -- %d \n",myBounds.right-myBounds.left,myBounds.bottom-myBounds.top);
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
			me->tryAttach();
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
				func("Setting volume to %f for channel %d",((float)vol)/100,me->chIndex);
				me->jmix->set_volume(me->chIndex,((float)vol)/100); 
			}
			break;
		case MENU_REMOVE_CMD:
			me->plRemoveSelection();
			break;
		case OPEN_FILE_CMD:
			err = OpenFileWindow(me->window);
			break;
		case OPEN_URL_CMD:
		//	err = OpenUrlWindow();
			break;
		case NEWC_CMD:
			me->parent->new_channel();
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
			FSRefMakePath (&fileRef,path,2048);
			if(!me->jmix->add_to_playlist(me->chIndex,path)) {
				me->msg->warning("Can't add %s to playList",path);
			}
		}
	}

	return anErr;
} // OpenDocument
