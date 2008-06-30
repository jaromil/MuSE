
#include "cdkbbox.h"
#include "debug.h"

CDKButtonbox::CDKButtonbox()
{
};

void CDKButtonbox::draw()
{
	drawCDKButtonbox (buttonbox, box);
};

void CDKButtonbox::setparm(char * title, char ** buttons, int xpos, int ypos, int height, int width)
{
	buttonbox = newCDKButtonbox (
			cdkscreen,
			xpos,
			ypos,
			height,
			width,
			title,
			rows,
			cols,
			buttons,
			buttonCount,
			highlight,
			box,
			0
			);

}

void CDKButtonbox::setscreen (CDKSCREEN *screen)
{
	cdkscreen = screen;
}

void CDKButtonbox::activate()
{
	activateCDKButtonbox (buttonbox, 0);
}

CDKButtonbox::~CDKButtonbox()
{
	warning ("buttonbox destructor");
	//destroyCDKButtonbox(buttonbox);
};

