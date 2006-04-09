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
 
 #ifndef __BUFFER_INSPECTOR_H__
 #define __BUFFER_INSPECTOR_H__

#include <jutils.h>
#include <carbon_common.h>
#include <carbon_message.h>
#include <jmixer.h>


class BufferInspector {
	public:
		BufferInspector(WindowRef mainWin,IBNibRef nib,Stream_mixer *mix);
		~BufferInspector();
		void show();
		void hide();
		void scanChannels();
		bool selectInput();
		bool selectOutput();
		bool setInput();
		bool setOutput();
		void run();
		
		CarbonMessage *msg;
		WindowRef window;
		
	private:
		bool attach();
		bool detach();
		void setupControls();
		
		IBNibRef nibRef;
		WindowRef parent;
		ControlRef iSelect;
		ControlRef iBar;
		ControlRef iStatus;
		ControlRef oSelect;
		ControlRef oBar;
		ControlRef oStatus;
		Stream_mixer *jmix;
		MenuHandle iMenu;
		MenuHandle oMenu;
		int inIdx;
		int outIdx;
	protected:
};

#endif
