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

#ifndef __CARBON_CHANNEL_H__
#define __CARBON_CHANNEL_H__
#include <Carbon/Carbon.h>

#include <jmixer.h>
#include <jutils.h>
#include "carbon_common.h"
#include "carbon_message.h"
#include <playlist.h>
#include "playlist_manager.h"

#define CARBON_MAX_PLAYLIST_ENTRIES 32000
				
class CARBON_GUI;
class CarbonChannel;

/* this struct is used when connecting two channel windows */
typedef struct AttractedChannel
{
	CarbonChannel *channel;
	unsigned char position;
#define ATTACH_LEFT 0
#define ATTACH_RIGHT 1
} AttractedChannel;

class CarbonChannel {
	public:
		CarbonChannel(Stream_mixer *mix,CARBON_GUI *gui,IBNibRef nib,unsigned int chan);
		~CarbonChannel();
		void setLCD(char *lcd); /* set lcd text message */
		void setPos(int pos); /* set position on the seek bar */
		
		void run(); /* channel main loop ... called every 2 ticks (about 1/60 of a second) */
		void close ();
		
		/* playlist related methods */
		bool plAdd(char *txt);
		void plSelect(int row); /* select the item at row in the playlist box */
		void plMove(int from,int to); /* move an item inside playlist */
		void plRemove(int pos); /* remove an entry in the current playlist */
		void plRemoveSelection(); /* remove selected item from playlist */
		bool plUpdate(); /* update playlist box control to reflect the underlying LinkList one */
		bool plSave(int mode); /* mode == 0  => create a new playlist, mode == 1 => update the loaded one */
		bool plLoad(int idx); /* load playlist at index idx (if present) */
		bool plDelete(int idx); /* remove an entire playlist from the saved ones */
		void plSaveDialog(); /* show the save dialog for a playlist */
		void plCancelSave(); /* abort saving of a playlist (while showing the dialog) */
		char *loadedPlaylist(); //< returns the name of the loadedPlaylist
		MenuRef plGetMenu(); /* get contextual menu for the selected item in the playlist */
		
		void openUrlDialog(); /* show the openUrl dialog to load an item in the playlist */
		void openFileDialog(); /* show the openFile dialog to load an item in the playlist */
		void tryOpenUrl(); /* try to open given url */
		void cancelOpenUrl(); /* abort opening of given url */
		void updatePlaymode(); 
		void setVol(int vol);
		void crossFade(int fadeVal);
		
		/* playing related methods */
		void play();
		void stop();
		void pause();
		void prev();
		void next();
		void seek(int pos);
		
		/* magnetic methods */
		void redrawFader();
		bool checkNeighbours();
		void attractNeighbour();
		void stopAttracting();
		void doAttach();
		void gotAttached(CarbonChannel *);
		void startResize();
		void stopResize();
		void stopFading();
		bool attached();
		bool resizing();
		bool slave();
		void activate();
				
		WindowRef window; /* channel window */
		WindowRef fader; /* fader window (drawer */
		WindowRef parentWin; /* a reference to the main window */
		WindowRef openUrlWindow; /* the openUrl dialog */
		WindowRef savePlaylistWindow; /* the savePlaylist dialog */
		WindowGroupRef faderGroup; /* window group used when connecting two channel windows */
		Stream_mixer *jmix; /* reference to Stream_mixer object (MuSE core) */
		CARBON_GUI *parent; /* reference to the main GUI object */
		AttractedChannel neigh; /* structure that describe the channel we are connected to (when fading) */
		/* window controls */
		ControlRef playListControl;
		ControlRef faderControl;
		ControlRef seekControl;
		ControlRef volControl;
		Playlist *playList;
		
		unsigned int chIndex; /* our index in the Stream_mixer inchannel array */
		CarbonMessage *msg; /* to handle notify and error messages */
		Channel *inChannel; /* the underlying Channel object */
		
	private:
		void lock() { pthread_mutex_lock(&_mutex); };
		void unlock() { pthread_mutex_unlock(&_mutex); };
		
		void activateMenuBar(); /* explicitly activate contextual menubar */
		void plSetup();
		int getNextPlayListID();
		void setupOpenUrlWindow();
		void setupFaderWindow();
		void setupSavePlaylistWindow();
		void updateSelectedSong(int row);
		void updatePlaylistControls();

		bool isDrawing;
		bool isResizing;
		bool isSlave;
		bool isAttached;
		
		pthread_mutex_t _mutex;
		
		IBNibRef nibRef;
		MenuRef plMenu;
		MenuRef plEntryMenu;
		MenuRef loadMenu;
		MenuRef deleteMenu;
		MenuRef saveMenu;
		EventLoopTimerRef updater;
		EventHandlerRef windowEventHandler;
		EventHandlerRef playListEventHandler;
		char lcd[255];
		AEEventHandlerUPP openHandler;
		int plDisplay;
		int status;
#define CC_STOP	0
#define CC_PLAY	1
#define CC_PAUSE 3
		int savedStatus;
		PlaylistManager *plManager;
		int loadedPlaylistIndex;
		int runningPlaylistIndex;
		bool plLoaded;
		int _seek;
		
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

/* 
 * XXX - A fake timer to workaround a strange Quartz behaviour with our multithreaded engine ...
 * read comments in the implementation for more details 
 */
void ChannelLoop(EventLoopTimerRef inTimer,void *inUserData); 

/****************************************************************************/
/* Event and Command handlers  (carbon callbacks)*/
/****************************************************************************/

/* channel window event handler callback */
static OSStatus 
	ChannelEventHandler (EventHandlerCallRef nextHandler, EventRef event, void *userData);
 
/* playlist box (aka databrowser) event handler callback */
static OSStatus 
	DataBrowserEventHandler(EventHandlerCallRef nextHandler, EventRef event, void *userData);
	
/* channel window command handler callback (handles clicks on player buttons and so on) */ 
static OSStatus 
	ChannelCommandHandler (EventHandlerCallRef nextHandler, EventRef event, void *userData);
	
/* fader command handler callback (handles fading between two channels) */
static OSStatus 
	FaderCommandHandler (EventHandlerCallRef nextHandler, EventRef event, void *userData);
	
/****************************************************************************/
/* DataBrowser (playlist)  handlers (carbon callbacks)*/
/****************************************************************************/
//OSErr ForceDrag (Point *mouse,SInt16 *modifiers,void *userData,DragRef theDrag);

/* callback called when adding an item to a drag...here we add a flavour to hanle dragging
 * of items between channels */
Boolean AddDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item,DragItemRef *itemRef);

/* callback function that gets and sets properties for individual items in the playlist */
OSStatus HandlePlaylist (ControlRef browser,DataBrowserItemID itemID,
	DataBrowserPropertyID property,DataBrowserItemDataRef itemData,Boolean changeValue);

/* callback to handle drags on the playlist box */
Boolean HandleDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item);

/* callback function that checks if playlist box can accept a specific drag */
Boolean CheckDrag (ControlRef browser,DragRef theDrag,DataBrowserItemID item);

/* callback function to handle user selction of items in the playlist box */
void HandlePlaylistEvents (ControlRef browser,DataBrowserItemID item,
   DataBrowserItemNotification message);

/* callback function to handle user requests of a contextual menus on an item in the playlist box */
void GetPLMenu (ControlRef browser,MenuRef *menu,UInt32 *helpType,
	CFStringRef *helpItemString, AEDesc *selection);
/*
void DrawPLItem (ControlRef browser,DataBrowserItemID item,DataBrowserPropertyID property,
   DataBrowserItemState itemState, const Rect *theRect,SInt16 gdDepth, Boolean colorDevice);


void SelectPLMenu(ControlRef browser,MenuRef menu,UInt32 selectionType,
	SInt16 menuID,MenuItemIndex menuItem);
*/	
void RemovePlaylistItem (DataBrowserItemID item,DataBrowserItemState state,void *clientData);

/* callback function to handle user changes in the fader controlbar */
void FaderHandler (ControlRef theControl, ControlPartCode partCode);
/* callback function to handle user changes in the seek controlbar */
void SeekHandler (ControlRef theControl, ControlPartCode partCode);

/****************************************************************************/
/* OpenDocument callbacks (called when opening from dialog) */
/****************************************************************************/

static OSErr OpenFile(EventRef event,CarbonChannel *me);

/* the openUrl command handler for the openUrl dialog handled internally in the channel object */
static OSStatus OpenUrlCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);
	
static OSStatus SavePlaylistCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData);
#endif
