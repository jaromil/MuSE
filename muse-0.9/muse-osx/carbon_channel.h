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

#ifndef __CARBON_CHANNEL_H__
#define __CARBON_CHANNEL_H__
#include <Carbon/Carbon.h>

#include <jmixer.h>
#include <jutils.h>
#include "carbon_common.h"
#include "carbon_message.h"
#include <playlist.h>
#define CARBON_MAX_PLAYLIST_ENTRIES 32000
				
class CARBON_GUI;
class CarbonChannel;

typedef struct {
	CarbonChannel *channel;
	unsigned char position;
#define ATTACH_LEFT 0
#define ATTACH_RIGHT 1
} AttractedChannel;

class CarbonChannel {
	public:
		CarbonChannel(Stream_mixer *mix,CARBON_GUI *gui,IBNibRef nib,unsigned int chan);
		~CarbonChannel();
		bool add_playlist(char *txt) { return plAdd(txt); } /* just an accessor to support muse API */
		void setLCD(char *lcd);
		void setPos(int pos);
		void activateMenuBar();
		void close ();
		
		/* playlist related methods */
		bool plAdd(char *txt);
		void plSetup();
		void plSelect(int row);
		void plMove(int from,int to);
		void plRemove(int pos);
		void plRemoveSelection();
		bool plUpdate();
		MenuRef plGetMenu();
		
		/* magnetic methods */
		bool checkNeighbours();
		void attractNeighbour();
		void stopAttracting();
		void doAttach();
		void redrawFader();
		void gotAttached(CarbonChannel *);
		void startResize();
		void stopResize();
		void stopFading();
		bool attached();
		bool resizing();
		bool slave();
		
		void setVol(int vol);
		void crossFade(int fadeVal);
		
		WindowRef window;
		WindowRef fader;
		WindowRef parentWin;
		WindowRef openUrl;
		WindowGroupRef faderGroup;
		Stream_mixer *jmix;
		CARBON_GUI *parent;
		AttractedChannel neigh;
		ControlRef playListControl;
		ControlRef faderControl;
		Playlist *playList;
		unsigned int chIndex;
		CarbonMessage *msg;
		Channel *inChannel;
		int seek;

	private:
		bool isDrawing;
		bool isResizing;
		bool isSlave;
		bool isAttached;
		
		IBNibRef nibRef;
		MenuRef plMenu;
		MenuRef plEntryMenu;
		EventLoopTimerRef updater;
		EventHandlerRef windowEventHandler;
		EventHandlerRef playListEventHandler;
		int getNextPlayListID();
		char lcd[255];
		AEEventHandlerUPP openHandler;
	protected:
};


/*
OSErr MyDragTrackingHandler (   
   DragTrackingMessage message, 
   WindowRef theWindow,   
   void * handlerRefCon,  
   DragRef theDrag);
   
OSErr MyDragReceiveHandler (    
   WindowRef theWindow,   
   void * handlerRefCon,    
   DragRef theDrag);
*/
void channelLoop(EventLoopTimerRef inTimer,void *inUserData);
/****************************************************************************/
/* Event handlers  (carbon callbacks)*/
/****************************************************************************/
static OSStatus ChannelEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);

static OSStatus dataBrowserEventHandler(
	EventHandlerCallRef nextHandler, EventRef event, void *userData);
	
static OSStatus channelCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);
	
static OSStatus faderCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);
	
/****************************************************************************/
/* DataBrowser (playlist)  handlers (carbon callbacks)*/
/****************************************************************************/
OSErr ForceDrag (Point *mouse,SInt16 *modifiers,void *userData,DragRef theDrag);

Boolean AddDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item,DragItemRef *itemRef);

OSStatus HandlePlaylist (ControlRef browser,DataBrowserItemID itemID,
	DataBrowserPropertyID property,DataBrowserItemDataRef itemData,Boolean changeValue);

Boolean HandleDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item);

Boolean CheckDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item);

void HandleNotification (ControlRef browser,DataBrowserItemID item,
   DataBrowserItemNotification message,DataBrowserItemDataRef itemData);

void GetPLMenu (ControlRef browser,MenuRef *menu,UInt32 *helpType,
	CFStringRef *helpItemString, AEDesc *selection);
/*
void SelectPLMenu(ControlRef browser,MenuRef menu,UInt32 selectionType,
	SInt16 menuID,MenuItemIndex menuItem);
*/	
void RemovePlaylistItem (DataBrowserItemID item,DataBrowserItemState state,void *clientData);

void faderHandler (ControlRef theControl, ControlPartCode partCode);

/****************************************************************************/
/* OpenDocument callbacks (called when opening from dialog */
/****************************************************************************/

static OSErr OpenFile(EventRef event,CarbonChannel *me);
	
#endif
