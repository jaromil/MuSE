/*
    Copyright (C) 1999-2002  Alexander König
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
    File: tX_knobloader.c
 
    Description: This code loads the knob-images required for tX_dial widget.
*/

#include <gtk/gtk.h>
#include "tX_knobloader.h"

#include "xpm/smallknob/knob0.xpm"
#include "xpm/smallknob/knob1.xpm"
#include "xpm/smallknob/knob2.xpm"
#include "xpm/smallknob/knob3.xpm"
#include "xpm/smallknob/knob4.xpm"
#include "xpm/smallknob/knob5.xpm"
#include "xpm/smallknob/knob6.xpm"
#include "xpm/smallknob/knob7.xpm"
#include "xpm/smallknob/knob8.xpm"
#include "xpm/smallknob/knob9.xpm"
#include "xpm/smallknob/knob10.xpm"
#include "xpm/smallknob/knob11.xpm"
#include "xpm/smallknob/knob12.xpm"
#include "xpm/smallknob/knob13.xpm"
#include "xpm/smallknob/knob14.xpm"
#include "xpm/smallknob/knob15.xpm"
#include "xpm/smallknob/knob16.xpm"
#include "xpm/smallknob/knob17.xpm"
#include "xpm/smallknob/knob18.xpm"
#include "xpm/smallknob/knob19.xpm"
#include "xpm/smallknob/knob20.xpm"
#include "xpm/smallknob/knob21.xpm"
#include "xpm/smallknob/knob22.xpm"
#include "xpm/smallknob/knob23.xpm"
#include "xpm/smallknob/knob24.xpm"
#include "xpm/smallknob/knob25.xpm"
#include "xpm/smallknob/knob26.xpm"
#include "xpm/smallknob/knob27.xpm"
#include "xpm/smallknob/knob28.xpm"
#include "xpm/smallknob/knob29.xpm"
#include "xpm/smallknob/knob30.xpm"
#include "xpm/smallknob/knob31.xpm"
#include "xpm/smallknob/knob32.xpm"
#include "xpm/smallknob/knob33.xpm"
#include "xpm/smallknob/knob34.xpm"
#include "xpm/smallknob/knob35.xpm"
#include "xpm/smallknob/knob36.xpm"
#include "xpm/smallknob/knob37.xpm"
#include "xpm/smallknob/knob38.xpm"
#include "xpm/smallknob/knob39.xpm"
#include "xpm/smallknob/knob40.xpm"
#include "xpm/smallknob/knob41.xpm"
#include "xpm/smallknob/knob42.xpm"
#include "xpm/smallknob/knob43.xpm"
#include "xpm/smallknob/knob44.xpm"
#include "xpm/smallknob/knob45.xpm"
#include "xpm/smallknob/knob46.xpm"
#include "xpm/smallknob/knob47.xpm"
#include "xpm/smallknob/knob48.xpm"
#include "xpm/smallknob/knob49.xpm"

char ** knob_pixs[MAX_KNOB_PIX]={
	 knob0_xpm,
	 knob1_xpm,
	 knob2_xpm,
	 knob3_xpm,
	 knob4_xpm,
	 knob5_xpm,
	 knob6_xpm,
	 knob7_xpm,
	 knob8_xpm,
	 knob9_xpm,
	 knob10_xpm,
	 knob11_xpm,
	 knob12_xpm,
	 knob13_xpm,
	 knob14_xpm,
	 knob15_xpm,
	 knob16_xpm,
	 knob17_xpm,
	 knob18_xpm,
	 knob19_xpm,
	 knob20_xpm,
	 knob21_xpm,
	 knob22_xpm,
	 knob23_xpm,
	 knob24_xpm,
	 knob25_xpm,
	 knob26_xpm,
	 knob27_xpm,
	 knob28_xpm,
	 knob29_xpm,
	 knob30_xpm,
	 knob31_xpm,
	 knob32_xpm,
	 knob33_xpm,
	 knob34_xpm,
	 knob35_xpm,
	 knob36_xpm,
	 knob37_xpm,
	 knob38_xpm,
	 knob39_xpm,
	 knob40_xpm,
	 knob41_xpm,
	 knob42_xpm,
	 knob43_xpm,
	 knob44_xpm,
	 knob45_xpm,
	 knob46_xpm,
	 knob47_xpm,
	 knob48_xpm,
	 knob49_xpm,	 
	};

GdkPixmap *knob_pixmaps[MAX_KNOB_PIX];

void load_knob_pixs(GtkWidget *wid_for_style)
{
	int i;
	GdkBitmap *mask;
	GtkStyle *style;
	
	style = gtk_widget_get_style(wid_for_style);
	
	for (i=0; i<MAX_KNOB_PIX; i++)
	{
		knob_pixmaps[i]=gdk_pixmap_create_from_xpm_d(wid_for_style->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) knob_pixs[i]);
	}
}

