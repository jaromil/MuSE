#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <utils.h>
#include <xpm/asbemuse.xpm>

GtkWidget*
about_win(void)
{


  GtkWidget *window1;
  GtkWidget *aboutframe;
  GtkWidget *fixed1;
  GtkWidget *pixmap1;
  GtkWidget *label1;
  GtkWidget *scrolledwindow1;
  GtkWidget *text1;

  window1 = gtk_window_new (GTK_WINDOW_DIALOG);
  gtk_object_set_data (GTK_OBJECT (window1), "window1", window1);
  gtk_widget_set_usize (window1, 550, 297);
  gtk_window_set_title (GTK_WINDOW (window1), "MuSE 0.8.1 codename \"Z0RZ0RZ\"");
  gtk_window_set_policy (GTK_WINDOW (window1), FALSE, FALSE, FALSE);

  aboutframe = gtk_frame_new ("about MuSE : the Multiple Streaming Engine");
  gtk_widget_ref (aboutframe);
  gtk_object_set_data_full (GTK_OBJECT (window1), "aboutframe", aboutframe,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (aboutframe);
  gtk_container_add (GTK_CONTAINER (window1), aboutframe);
  gtk_container_set_border_width (GTK_CONTAINER (aboutframe), 2);

  fixed1 = gtk_fixed_new ();
  gtk_widget_ref (fixed1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "fixed1", fixed1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fixed1);
  gtk_container_add (GTK_CONTAINER (aboutframe), fixed1);

  GdkPixmap *pix;
  GdkBitmap *mask;
  GtkStyle *style;
  style=gtk_widget_get_style(window1);
  gtk_widget_realize(window1);
  pix = gdk_pixmap_create_from_xpm_d(window1->window, &mask, &style->bg[GTK_STATE_NORMAL], asbemuse);
  pixmap1 = gtk_pixmap_new(pix,mask);
  gtk_widget_ref (pixmap1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "pixmap1", pixmap1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pixmap1);
  gtk_fixed_put (GTK_FIXED (fixed1), pixmap1, 361, 0);
  //  gtk_widget_set_usize (pixmap1, 330, 276);
  gtk_widget_set_usize (pixmap1, 189, 270);

  label1 = gtk_label_new ("development team for this release:\n"
			  "\n"
			  "jaromil aka Denis Rojo - main coder and mantainer\n"
			  "nightolo aka Antonino Radici - GTK user interface\n"
			  "rubik aka Luca Profico - console user interface\n"
			  "godog aka Filippo Giunchedi - docu and organizer\n");

  gtk_widget_ref (label1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label1", label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label1);
  gtk_fixed_put (GTK_FIXED (fixed1), label1, 0, 8);
  gtk_widget_set_uposition (label1, 0, 8);
  gtk_widget_set_usize (label1, 300, 103);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label1), 0.5, 0.1);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_ref (scrolledwindow1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "scrolledwindow1", scrolledwindow1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow1);
  gtk_fixed_put (GTK_FIXED (fixed1), scrolledwindow1, 0, 120);
  gtk_widget_set_uposition (scrolledwindow1, 0, 120);
  gtk_widget_set_usize (scrolledwindow1, 347, 153);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  text1 = gtk_text_new (NULL, NULL);
  gtk_widget_ref (text1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "text1", text1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), text1);
  gtk_widget_set_usize (text1, 347, 150);
  gtk_text_insert (GTK_TEXT (text1), NULL, NULL, NULL,
		   "MuSE is an application for the mixing, encoding, and\n"
		   "streaming of sound: is an engine that can simultaniously\n"
		   "mix up to 6 separate MP3 or OggVorbis audio files from\n"
		   "the hard drive or the network, where each channel of\n"
		   "audio can be dynamicly adjusted for speed and volume\n"
		   "plus a soundcard line-in channel. The resulting stream\n"
		   "can be played locally on the sound card and/or\n"
		   "encoded as an mp3 network stream to an icecast or\n"
		   "shoutcast server (ready to be mixed and played\n"
		   "again by other muses... ;)\n"
		   "\n"
		   "MuSE as it is now would have never existed without\n"
		   "the contributions of (in reverse chronological order):\n"
		   "= Matteo Nastasi aka Mop ( alternativeoutput.it )\n"
		   "= Lobo for herbivore and free open air open source\n"
		   "= PUBLIC VOICE Lab ( pvl.at ) support for development\n"
		   "= SERVUS.AT trusting this project since its beginning\n"
		   "= Asbesto Molesto ( freaknet.org ) testing and docu\n"
		   "= Alex, Rasty and Martinez ( maria libera! ) good vibes\n"
		   "= August Black ( aug.ment.org ) original GUI scheme\n"
		   "= Markus Seidl ( funda.ment.org ) vision of such a tool\n"
		   "\n"
		   "Also waves and thanks to:\n"
		   "voyager, void, blicero, sandolo, eni,\n"
		   "the Freaknet Medialab, the ASCII squat,\n"
		   "henk, the imc-audio collective, jeff,\n"
		   "the LOA hacklab, the TPO, bundes & didi,\n"
		   "indymedia italy, neural.it, autistici.org,\n"
		   "Mag-One, radio Ondarossa, bomboclat, newmark\n"
		   "c1cc10, vanguardist, janki, godog, kysucix\n"
		   "and all the others i'm forgetting here!\n"
		   "\n"
		   "\nMuSE redistributes statically, the following libraries:\n= libmpeg by Woo-jae Jung and Mikael Hedin\n= libshout by Jack Moffit and others\n= libcdk by Mike Glover\n\n\nMuSE can link dinamically to the following libraries:\n= libogg, libvorbis, libvorbisfile - www.xiph.org\n= glib, libgdk, libgtk - www.gtk.org\n= liblame - www.mp3dev.org/mp3\n= libX11, libXext - www.xfree86.org\n= other common GNU libraries\n\n\nMuSE sourcecode also got inspirations and code\nsnippets from the stream mixer sourcecode by\nScott Manley, the buffered FIFO pipe source\nby Charles Samuels, the GTK knob widget by\nAlexander Koenig, GNOME icons by Jakub Steiner\n\n"
		   "MuSE Streamer is copyleft (c)\n"
		   "2000-2003 by Denis \"jaromil\" Rojo\n"
		   "\n"
		   "MuSE's GTK+ GUI is copyleft (c)\n"
		   "2001-2003 by Antonino \"nightolo\" Radici\n"
		   "\n"
		   "MuSE's NCURSES GUI is copyleft (c)\n"
		   "2002-2003 by Luca \"rubik\" Profico\n"
		   "\n"
		   "DSP resampling routines are copyleft (c)\n"
		   "2002 by Matteo \"MoP\" Nastasi\n"
		   "\n"
		   "MuSE's first GUI scheme is copyleft (c)\n"
		   "2000-2001 by August Black\n"
		   "\n"
		   "part of the included code is copyright by the\n"
		   "respective authors, please refer to the supplied\n"
		   "sourcecode for further informations.\n"
		   "\n-----------------------------------------------------------\n"
		   "\nThis source code is free software; you can redistribute\nit and/or modify it under the terms of the GNU Public\nLicense as published by the Free Software Foundation;\neither version 2 of the License, or (at your option) any\nlater version.\n\nThis source code is distributed in the hope that it will\nbe useful, but WITHOUT ANY WARRANTY; without\neven the implied warranty of MERCHANTABILITY or\nFITNESS FOR A PARTICULAR PURPOSE.\nPlease refer to the GNU Public License for more details.\n\nYou should have received a copy of the GNU Public\nLicense along with this source code; if not, write to:\nFree Software Foundation, Inc., 675 Mass Ave,\nCambridge, MA 02139, USA\n\n\n", -1);

  gtk_widget_show_all(window1);

  return window1;
}
