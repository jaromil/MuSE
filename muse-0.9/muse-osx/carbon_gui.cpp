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


CARBON_GUI::CARBON_GUI(int argc, char **argv, Stream_mixer *mix)
  : GUI(argc,argv,mix) {
  	jmix = mix;
	if(!mix->chan[0]) mix->create_channel(0);
	// Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
  //  require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
  //  require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
	

	//err = InstallEventHandler(GetWindowEventTarget(window),close,1,&evtClose,this,NULL);
	if(err != noErr) { 
		/* TODO - Warning messages */
	}
	
	
	BringToFront(window);
	// The window was created hidden so show it.
    ShowWindow( window );
	unsigned int i;
	for (i=0;i<MAX_CHANNELS;i++) {
		if(jmix->chan[i]) { 
				CARBON_CHANNEL *newChan = new CARBON_CHANNEL(jmix,window,nibRef,i);
				channel[i] = newChan;
				if(i > 0) {
					RepositionWindow(channel[i]->window,channel[i-1]->window,kWindowCascadeOnParentWindow);
				}
				//else {
				//	RepositionWindow(channel[i],window,kWindowCascadeOnParentWindow);
				//}
		}
	}

/*    
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
*/
	//return this;
//  gtkgui_init(argc,argv,mix);
 /// for(int i=0;i<MAX_CHANNELS;i++) {
 //   new_pos[i] = false;
 //   new_lcd[i] = false;
 //   new_sel[i] = 0;
 // }
}

CARBON_GUI::~CARBON_GUI() { 
// We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
}

void CARBON_GUI::run() {
	while(!quit) {
	}
 }

void CARBON_GUI::set_title(char *txt) {
//  gtkgui_set_maintitle(txt);
}
 
void CARBON_GUI::set_status(char *txt) {
//  gtkgui_set_statustext(txt);
}

void CARBON_GUI::add_playlist(unsigned int ch, char *txt) {
 // gtkgui_add_into_pl(ch,txt);
}

void CARBON_GUI::sel_playlist(unsigned int ch, int row) {
//  new_sel[ch] = row;
}

bool CARBON_GUI::meter_shown() { }//return vu_status; }
/*
void CARBON_GUI::stop() {
	QuitEventLoop(eventLoop);
}*/
/*void CARBON_GUI::start() {
	printf("\n\n\n\n\n\n\nKAAAAAKKAAAa\n\n\n\n");
	MPCreateQueue(&mpQueue);
	MPCreateTask( startCarbon,this,0,mpQueue,this,NULL,kMPCreateTaskValidOptionsMask,&threadID );
}

OSStatus startCarbon(void *arg) {
	//CFRunLoopRef myLoop = CFRunLoopGetCurrent ();
	//CFRunLoopRun();
	printf("\n\n\n\nFATTO\n\n\n\n");
	((CARBON_GUI *)arg)->run();
	return noErr; 
}*/
