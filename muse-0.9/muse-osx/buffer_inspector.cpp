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

#include "buffer_inspector.h"
#include "dev_sound.h"
static OSStatus BufferInspectorEventHandler (
	EventHandlerCallRef nextHandler, EventRef event, void *userData);

const ControlID iSelID = { CARBON_GUI_APP_SIGNATURE, BI_INPUT_SELECTOR };
const ControlID iBarID = { CARBON_GUI_APP_SIGNATURE, BI_INPUT_BAR };
const ControlID iStatusID = { CARBON_GUI_APP_SIGNATURE, BI_INPUT_STATUS };
const ControlID oSelID = { CARBON_GUI_APP_SIGNATURE, BI_OUTPUT_SELECTOR };
const ControlID oBarID = { CARBON_GUI_APP_SIGNATURE, BI_OUTPUT_BAR };
const ControlID oStatusID = { CARBON_GUI_APP_SIGNATURE, BI_OUTPUT_STATUS };

const EventTypeSpec windowEvents[] = {
	{ kEventClassWindow, kEventWindowClose },
	{ kEventClassCommand, kEventCommandProcess },
	{ kEventClassWindow, kEventWindowActivated }
};

BufferInspector::BufferInspector(WindowRef mainWin,IBNibRef nib,Stream_mixer *mix) {
	OSStatus err;
	parent = mainWin;
	nibRef = nib;
	jmix = mix;
	
	inIdx = -1;
	outIdx = -1;
	
	msg = new CarbonMessage(nibRef);
	
	err = CreateWindowFromNib(nibRef,CFSTR("BufferInspector"),&window);
	if(err != noErr)
		msg->error("Can't create te BufferInspector window (%d)!!",err);
		
	setupControls();
	
	err = InstallEventHandler(GetWindowEventTarget(window),NewEventHandlerUPP(BufferInspectorEventHandler),3,windowEvents,this,NULL);
	if(err != noErr)
		msg->error("Can't install event handler for BufferInspector (%d)!!",err);
		
}

void BufferInspector::setupControls() {
	OSStatus err;
	Size sz;
	
	err = GetControlByID(window,&iSelID,&iSelect);
	if(err != noErr)
		msg->error("Can't get InputSelector ControlRef (%d)!!",err);
	err = GetControlByID(window,&iBarID,&iBar);
	if(err != noErr)
		msg->error("Can't get InputBar ControlRef (%d)!!",err);
	err = GetControlByID(window,&iStatusID,&iStatus);
	if(err != noErr)
		msg->error("Can't get InputStatus ControlRef (%d)!!",err);
	err = GetControlByID(window,&oSelID,&oSelect);
	if(err != noErr)
		msg->error("Can't get OutputSelector ControlRef (%d)!!",err);
	err = GetControlByID(window,&oBarID,&oBar);
	if(err != noErr)
		msg->error("Can't get OutputBar ControlRef (%d)!!",err);
	err = GetControlByID(window,&oStatusID,&oStatus);
	if(err != noErr)
		msg->error("Can't get OutputStatus ControlRef (%d)!!",err);

	err = GetControlData(iSelect,kControlButtonPart,kControlPopupButtonMenuHandleTag,sizeof(MenuHandle),&iMenu,&sz);
	if(err != noErr)
		msg->error("Can't get iMenu (%d)!!",err);
	err = GetControlData(oSelect,kControlButtonPart,kControlPopupButtonMenuHandleTag,sizeof(MenuHandle),&oMenu,&sz);
	if(err != noErr)
		msg->error("Can't get oMenu (%d)!!",err);

}

void BufferInspector::run() {
	SInt32 val;
	SInt32 max;
	if(inIdx >= 0) {
		if(inIdx == 0) {
		}
		else if(jmix->chan[inIdx-1]) {
			val = jmix->chan[inIdx-1]->erbapipa->space() * 100 / jmix->chan[inIdx-1]->erbapipa->size();
			max = GetControl32BitMaximum(iStatus);
			val = max - (max / 100 * val);
			SetControl32BitValue(iStatus,val);
		}
		else {
			inIdx = -1;
			SetControl32BitValue(iStatus,0);
		}
	}
	else {
		inIdx = -1;
		SetControl32BitValue(iStatus,0);
	}
	if(outIdx >= 0) {
		if(outIdx == 0) {
			PaDevInfo *dev;
			dev = &jmix->snddev->output_device;
			val = dev->pipe->space() * 100 / dev->pipe->size();
			max = GetControl32BitMaximum(oStatus);
			val = max - (max / 100 * val);
			SetControl32BitValue(oStatus,val);
		}
		else {
			OutChannel *chan;
			chan = (OutChannel *)jmix->outchans.pick(outIdx);
			if(chan) {
				val = chan->erbapipa->space() * 100 / chan->erbapipa->size();
				max = GetControl32BitMaximum(oStatus);
				val = max - (max / 100 * val);
				SetControl32BitValue(oStatus,val);
			}
			else {
				outIdx = -1;
				SetControl32BitValue(oStatus,0);
			}
		}
	}
	else {
		outIdx = -1;
		SetControl32BitValue(oStatus,0);
	}
}

void BufferInspector::show() {
	scanChannels();
	ShowWindow(window);
}

void BufferInspector::hide() {
	HideWindow(window);
}

bool BufferInspector::attach() {
}

bool BufferInspector::detach() {
}

void BufferInspector::scanChannels() {
	OSStatus err = noErr;
	int i;
	UInt16 nItems;
	if(jmix->linein)
		ChangeMenuItemAttributes(iMenu,2,0,kMenuItemAttrDisabled);
	else
		ChangeMenuItemAttributes(iMenu,2,kMenuItemAttrDisabled,0);
	for (i=0; i<MAX_CHANNELS; i++) {
		if(jmix->chan[i]) {
			ChangeMenuItemAttributes(iMenu,i+3,0,kMenuItemAttrDisabled);
		}
		else {
			ChangeMenuItemAttributes(iMenu,i+3,kMenuItemAttrDisabled,0);
		}
	}
	nItems=CountMenuItems(oMenu);
	if(jmix->dspout)
		ChangeMenuItemAttributes(oMenu,2,0,kMenuItemAttrDisabled);
	else
		ChangeMenuItemAttributes(oMenu,2,kMenuItemAttrDisabled,0);
	if(nItems > 1) {
		err = DeleteMenuItems(oMenu,3,nItems-2);
	}
	for(i=1; i<= jmix->outchans.len(); i++) {
		MenuItemIndex newIdx;
		char oName[256];
		sprintf(oName,"outchannel %d",i);
		CFStringRef text=CFStringCreateWithCString(NULL,oName,0);
		AppendMenuItemTextWithCFString(oMenu,text,kMenuItemAttrUpdateSingleItem,0,&newIdx); //BI_SELECT_OUTPUT,&newIdx);
		CFRelease(text);
	}
}

bool BufferInspector::selectInput() {
	inIdx = GetControl32BitValue(iSelect)-2;
	return true;
}

bool BufferInspector::selectOutput() {
	outIdx = GetControl32BitValue(oSelect)-2;
	return true;
}

bool BufferInspector::setInput() {
}

bool BufferInspector::setOutput() {
}

/************************************************************
 * EVENT HANDLER 
 ************************************************************/
 static OSStatus BufferInspectorEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    OSStatus err = noErr;
    BufferInspector *me = (BufferInspector *)userData;
	switch (GetEventKind (event))	
    {
        case kEventWindowClose: 
			me->hide();
			return noErr;
            break;
		case kEventWindowActivated:
			me->scanChannels();
			break;
		case kEventCommandProcess:
			HICommand command;
			err = GetEventParameter(event, kEventParamDirectObject,
				typeHICommand, NULL, sizeof(HICommand), NULL, &command);
			if(err != noErr) me->msg->error("Can't get event parameter!!");
			switch (command.commandID)
			{
				case BI_SET_INPUT:
					me->setInput();
					break;
				case BI_SET_OUTPUT:
					me->setOutput();
					break;
				case BI_SELECT_INPUT:
					me->selectInput();
					break;
				case BI_SELECT_OUTPUT:
					me->selectOutput();
					break;
			}
		default:
            break;
    }
    return CallNextEventHandler(nextHandler,event);
}


