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


#ifndef __CREDITS_H__
#define __CREDITS_H__

#include <Carbon/Carbon.h>
#include <carbon_message.h>
#include <carbon_common.h>

/*
 * This class just implements the about window shown on user request 
 * It doesn't do anything special....just initialize message boxes when created
 */
class AboutWindow {
        public:
                AboutWindow(WindowRef mainWin,IBNibRef nib);
                ~AboutWindow();
				void show();
				void hide();
        private:
                WindowRef parent,window; /* me and my parent :P */
				WindowGroupRef aboutGroup; /* needed to handle window layering correctly :/ */
                ControlRef msgBox;
				ControlRef devBox;
				TXNObject msgText;
				TXNObject devText;
				IBNibRef nibRef;
				CarbonMessage *msg;
};


#endif
