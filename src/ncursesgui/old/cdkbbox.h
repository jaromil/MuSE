
#ifndef __RUB_BBOX_H_
#define __RUB_BBOX_H_

#include "cdkscreen.h"

extern "C"
{
#include <cdk/cdk.h>
}

class CDKButtonbox 
{
	protected:
		CDKBUTTONBOX *buttonbox;
		CDKSCREEN *cdkscreen;
		char **title;
		char **buttons;
	public:
		int buttonCount , rows, cols;
		chtype highlight;
		bool box;
		
		CDKButtonbox();
		~CDKButtonbox();
		void activate();
		void setscreen(CDKSCREEN *);
		void setparm(char * title, char ** buttons, int xpos, int ypos, int height, int width);
		void draw();
};

#endif
