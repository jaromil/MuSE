/* A NCURSES/CDK TUI (Text User Interface) for MuSE
 * Copyright (C) 2002 Luca 'rubik' Profico <rubik@olografix.org>
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
 *
 * $Id$
 *
 */

#include "slider.h"
#include "muse_console.h"

CDKSlider::CDKSlider()
{
};

CDKSlider::~CDKSlider()
{
	destroyCDKSlider (slider);
}

void CDKSlider::draw(void)
{
	drawCDKSlider (slider, box);
}

void CDKSlider::hide(void)
{
	eraseCDKSlider (slider);
}

void CDKSlider::setparm(char * title, char *label, int fieldwidth, int xpos, int ypos)
{
	i_curpos = highValue;
	slider = newCDKSlider (
			cdkscreen,
			xpos,
			ypos,
			title,
			label,
			A_REVERSE | COLOR_PAIR (29) | ' ', //fillercharacter
		        fieldwidth,
			currentValue,
			lowValue,
			highValue,
			1, //increment
			2, //fastincrement
			box, //box
			FALSE //shadow
			);

}

void CDKSlider::incr (void)
{
	injectCDKSlider (slider, '+');
}

void CDKSlider::decr (void)
{
	injectCDKSlider (slider, '-');
}

void CDKSlider::setscreen (CDKSCREEN *screen)
{
	cdkscreen = screen;
}

void CDKSlider::reg (void)
{
	registerCDKObject (cdkscreen, vSLIDER, slider);
}

void CDKSlider::unreg (void)
{
	unregisterCDKObject (vSLIDER, slider);
}

void CDKSlider::activate()
{
	activateCDKSlider (slider, 0);
}

void CDKSlider::set_pos (float pos)
{
	int rel;
	rel = (int) (pos * (highValue - lowValue)  + lowValue);
	if (i_curpos != rel)
	{
		setCDKSliderValue (slider, rel);
		i_curpos = rel;
	}
}

float CDKSlider::fpos (void)
{
	float i;
	i = ((float)(getCDKSliderValue(slider) - getCDKSliderLowValue(slider)) / 
			(float)(getCDKSliderHighValue(slider) - getCDKSliderLowValue(slider)));
	return i;
}

