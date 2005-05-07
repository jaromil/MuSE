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
 
#include <jutils.h>
#include "open_dialog.h"
#include "carbon_common.h"

static NavDialogRef gOpenFileDialog = NULL;

OSStatus OpenUrlWindow(WindowRef parent,WindowRef openWin) {
	ShowWindow(openWin);
}

OSStatus OpenFileWindow(WindowRef parent) {
	short				numTypes;
	OSType				typeList[20];
	OSType				fileType = '????';
	NavDialogRef		navDialog;
	
	// Open as many documents as the user wishes through Appleevents
	return OpenFileDialog( CARBON_GUI_APP_SIGNATURE, 0, NULL, &navDialog,parent );
} // DoOpenWindow

static pascal void MyPrivateEventProc( const NavEventCallbackMessage callbackSelector, 
   NavCBRecPtr callbackParms,NavCallBackUserData callbackUD )
{
	WindowRef parent = (WindowRef)callbackUD;
	switch ( callbackSelector )	{	
		case kNavCBEvent:
			switch (callbackParms->eventData.eventDataParms.event->what) {
				case updateEvt:
				case activateEvt:
//					HandleEvent(callbackParms->eventData.eventDataParms.event);
				break;
			}
		break;

		case kNavCBUserAction:
			if ( NavDialogGetUserAction(gOpenFileDialog) == kNavUserActionChoose )	{
				// This is an open files action, send an AppleEvent
				NavReplyRecord	reply;
				OSStatus		status;
				status = NavDialogGetReply( gOpenFileDialog, &reply );
				if ( status == noErr )
				{
					SendOpenEvent( reply.selection,parent );
					NavDisposeReply( &reply );
				}
			}
		break;
		
		case kNavCBTerminate:
			if ( callbackParms->context == gOpenFileDialog ) {
				NavDialogDispose( gOpenFileDialog );
				gOpenFileDialog = NULL;
			}
			
			// if after dismissing the dialog SimpleText has no windows open (so Activate event will not be sent) -
			// call AdjustMenus ourselves to have at right menus enabled
		//	if (FrontWindow() == nil) AdjustMenus(nil, true, false);
		break;
	}
}


static NavEventUPP GetPrivateEventUPP() {
	static NavEventUPP	privateEventUPP = NULL;				
	if ( privateEventUPP == NULL )	{
		privateEventUPP = NewNavEventUPP( MyPrivateEventProc );
	}
	return privateEventUPP;
}

static Handle NewOpenHandle(OSType applicationSignature, short numTypes, OSType typeList[]) {
	Handle hdl = NULL;
	
	if ( numTypes > 0 )	{
	
		hdl = NewHandle(sizeof(NavTypeList) + numTypes * sizeof(OSType));
	
		if ( hdl != NULL )	{
			NavTypeListHandle open		= (NavTypeListHandle)hdl;
			
			(*open)->componentSignature = applicationSignature;
			(*open)->osTypeCount		= numTypes;
			BlockMoveData(typeList, (*open)->osType, numTypes * sizeof(OSType));
		}
	}
	
	return hdl;
}

void TerminateDialog( NavDialogRef inDialog ) {
	NavCustomControl( inDialog, kNavCtlTerminate, NULL );
}

void TerminateOpenFileDialog() {
	if ( gOpenFileDialog != NULL ) 	{
		TerminateDialog( gOpenFileDialog );
	}
}


OSStatus OpenFileDialog(
	OSType applicationSignature, 
	short numTypes, 
	OSType typeList[], 
	NavDialogRef *outDialog,WindowRef parent )
{
	OSStatus theErr = noErr;
	if ( gOpenFileDialog == NULL )	{
		NavDialogCreationOptions	dialogOptions;
	
		NavGetDefaultDialogCreationOptions( &dialogOptions );
	
		dialogOptions.modality = kWindowModalityWindowModal;
		dialogOptions.parentWindow=parent;
		dialogOptions.clientName = CFStringCreateWithPascalString( NULL, LMGetCurApName(), GetApplicationTextEncoding());
		
		theErr = NavCreateChooseObjectDialog( &dialogOptions, GetPrivateEventUPP(), NULL, NULL, parent, &gOpenFileDialog );

		if ( theErr == noErr )	{
			theErr = NavDialogRun( gOpenFileDialog );
			if ( theErr != noErr )	{
				NavDialogDispose( gOpenFileDialog );
				gOpenFileDialog = NULL;
			}
		}
		
		if ( dialogOptions.clientName != NULL )	{
			CFRelease( dialogOptions.clientName );
		}
	}
	else {
		if ( NavDialogGetWindow( gOpenFileDialog ) != NULL ) {
			SelectWindow( NavDialogGetWindow( gOpenFileDialog ));
		}
	}
	
	if ( outDialog != NULL ) {
		*outDialog = gOpenFileDialog;
	}

	return NULL;
}


OSStatus SendOpenEvent( AEDescList list,WindowRef parent ) {
	OSStatus		err;
	EventRef		theEvent;
	err=CreateEvent (NULL,kCoreEventClass,kAEOpenDocuments,0,kEventAttributeUserEvent,&theEvent);
	SetEventParameter(theEvent,OPEN_DOCUMENT_DIALOG_PARAM,typeAEList,sizeof(AEDescList),&list);
	err=SendEventToEventTarget(theEvent,GetWindowEventTarget(parent));

	return err;
}
