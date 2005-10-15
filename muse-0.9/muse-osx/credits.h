/*
 *  credits.h
 *  muse-osx
 *
 *  Created by xant on 10/15/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __CREDITS_H__
#define __CREDITS_H__

#include <Carbon/Carbon.h>
#include <carbon_message.h>
#include <carbon_common.h>

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
