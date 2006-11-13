/* A Xml profile manager for MuSE - Multiple Streaming Engine
 * Copyright (C) 2002-2004 nightolo <night@autistici.org>
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

#ifndef XMLPROFILE_H
#define XMLPROFILE_H
typedef enum {
	STATE_NULL,
	STATE_NAME,
	STATE_HOST,
	STATE_PORT,
	STATE_MNT,
	STATE_STREAM_NAME,
	STATE_URL,
	STATE_DESCRIPTION,
	STATE_LOGINTYPE,
	STATE_PASSWORD } IceState;

/* OggState and LameState are the same thing */
typedef enum {
	STATE_NULL1,
	STATE_NAME1,
	STATE_QUALITY,
	STATE_MODE,
	STATE_BITRATE,
	STATE_FREQUENCY,
	STATE_FREQFIL,
	STATE_LOWPASS,
	STATE_HIGHPASS } EncState;

struct iceprof {
	gchar *name, *host, *port, *mnt;
	gchar *stream_name, *url, *desc, *logintype, *password;
};
struct encprof {
	gchar *name, *mode, *bitrate, *frequency;
	gchar *freqfil, *lowpass, *highpass;
	gdouble quality;
};
/* FIXME: DO NOT insert here private functions */

/* create dir if not present */
gboolean profile_init(void);
/* list profiles, fill a GList with load  */
/* load profile */
void profile_lame_load(void);
void profile_vorbis_load(void);
void profile_ice_load(void);
/* callback for reading */
/* start_element */
void enc_get_element(GMarkupParseContext *, const gchar *, const gchar **,
		const gchar **, gpointer, GError **);
void ice_get_element(GMarkupParseContext *, const gchar *, const gchar **,
		const gchar **, gpointer, GError **);

/* end_element */
void lame_end_element(GMarkupParseContext *, const gchar *,
		gpointer, GError **);
void vorbis_end_element(GMarkupParseContext *, const gchar *,
		gpointer, GError **);
void ice_end_element(GMarkupParseContext *, const gchar *,
		gpointer, GError **);
/* text */
void enc_get_value(GMarkupParseContext *, const gchar *, gsize,
		gpointer, GError **);
void ice_get_value(GMarkupParseContext *, const gchar *, gsize,
		gpointer, GError **);

/* write profile (FIXME: perform modify)*/
void profile_lame_write(void);
void profile_vorbis_write(void);
void profile_ice_write(void);
/* remove profile */
void profile_lame_remove(gchar *);
void profile_vorbis_remove(gchar *);
void profile_vorbis_remove(gchar *);


/* utility routine */
void profile_ice_free(struct iceprof *);
void profile_enc_free(struct encprof *);

#endif
