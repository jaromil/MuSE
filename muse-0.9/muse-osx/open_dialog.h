/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2005 xant <xant@dyne.org>
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

#ifndef __OPEN_DIALOG_H__
#define __OPEN_DIALOG_H__

OSStatus OpenUrlWindow(WindowRef parent,WindowRef openWin);
OSStatus OpenFileWindow(WindowRef parent);
OSStatus OpenFolderWindow(WindowRef parent);

OSStatus OpenDialog(OSType applicationSignature, short numTypes, 
	OSType typeList[], NavDialogRef *outDialog,WindowRef parent,unsigned int mode );
void TerminateOpenFileDialog();
void TerminateDialog( NavDialogRef inDialog );
static Handle NewOpenHandle(OSType applicationSignature, short numTypes, OSType typeList[]);
static NavEventUPP GetPrivateEventUPP();
static pascal void MyPrivateEventProc( const NavEventCallbackMessage callbackSelector, 
	NavCBRecPtr callbackParms,NavCallBackUserData callbackUD );
OSStatus SendOpenEvent( AEDescList list,WindowRef parent);
#endif
