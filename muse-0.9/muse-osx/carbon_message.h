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
 
#ifndef __CARBON_MESSAGE_H__
#define __CARBON_MESSAGE_H__

#include <Carbon/Carbon.h>
#include <jutils.h>


#define CM_ERROR 0
#define CM_ERROR_ID 600
#define CM_WARNING 1
#define CM_WARNING_ID 601
#define CM_NOTIFY 2
#define CM_NOTIFY_ID 602

#define CM_MSG_MAXLEN 256

#define CM_TYPE_MAX 2

class CarbonMessage {
	public:
		CarbonMessage(IBNibRef nib);
		~CarbonMessage();
		void error(const char *format, ... );
		void warning(const char *format, ... );
		void notify(const char *format, ... );
		
	private:
	protected:
		void run(CFStringRef windowName,SInt32 textId,const char *msg );
		void setText(const char *msg);
		ControlRef textControl;
		IBNibRef nibRef;
		unsigned char type;
		char text[CM_MSG_MAXLEN];
		WindowGroupRef msgGroup;
		
};


#endif

